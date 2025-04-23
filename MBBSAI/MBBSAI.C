/*****************************************************************************
 *   MBBSAI.C   V1.0                           AI Chat for MBBS              *
 *                                                                           *
 *   Copyright (C) 2025 Electron Wrangler.  All Rights Reserved.             *
 *                                                                           *
 *   Description:        A simple module to let your users chat              *
 *                       with OpenAI's ChatGPT.                              *
 *                                                                           *
 *                            - 3o8 2025-04-21                               *
 *                                                                           *
 *   See LICENSE file in solution root for terms and conditions.             *
 *                                                                           *
 *****************************************************************************/

#include "gcomm.h"
#include "majorbbs.h"
#include "MBBSAI.H"
#include "ChatWrapper_C.h"

// function declarations
GBOOL user_logon(VOID);              /* Logon routine                        */
GBOOL module_main(VOID);             /* Main module routine                  */         
VOID  user_deleted(CHAR* userid);    /* Delete account routine               */
VOID  store_user_record(CHAR* userid); /* Store user record in DAT file      */
VOID  display_user_records(VOID);    /* Display users in DAT file            */
VOID  system_shutdown(VOID);         /* Shutdown routine                     */    
VOID  end_mbbsai(VOID);              /* empty fcn to show EOF for debug      */
VOID  ai_chat(CHAR* usertext);                 /* Call the AI chat function            */
VOID  on_chunk(VOID);                /* output chunked response data         */ 
VOID  cleanup_mbbsai(VOID);          /* Dispose of open handle if needed     */
INT   show_message(VOID);            /* injoth routine                       */   

// module definition
INT usrstt;                          /* User state in this module            */
struct module MBBSAI={               /* module interface block               */
     "",                             /* name used to refer to this module    */
     user_logon,                     /* user logon supplemental routine      */
     module_main,                    /* input routine if selected            */
     NULL,                           /* status-input routine if selected     */
     show_message,                   /* "injoth" routine for this module     */
     NULL,                           /* user logoff supplemental routine     */
     NULL,                           /* hangup (lost carrier) routine        */
     NULL,                           /* midnight cleanup routine             */
     user_deleted,                   /* delete-account routine               */
     system_shutdown                 /* finish-up (sys shutdown) routine     */
};

// global variables
GBOOL display_logon_msg;             /* Display logon message?               */
HMCVFILE modmb;                      /* Module message file                  */
DFAFILE* moddat;                     /* Module data file                     */

static CHAT_HANDLE ai_handle;        /* Handle for the AI chat                */

struct moddat {                      /* Example module data file structure   */
    CHAR name[UIDSIZ];               /* userid                               */
};

// begin routines

void EXPORT
init__mbbsai(VOID)                   /* initialize Module                    */
{
    // register the module
    stzcpy(MBBSAI.descrp, gmdnam("MBBSAI.MDF"), MNMSIZ);
    usrstt = register_module(&MBBSAI);

    // open the message file
    modmb = opnmsg("MBBSAI.MCV");
    display_logon_msg = ynopt(DISPLOM);

    // open the data file
    moddat = dfaOpen("MBBSAI.DAT", sizeof(struct moddat), NULL);
    
    // Add message to audit log that module has been loaded
    shocst(spr("AI Chat for MBBS v10"),"Loaded version 1.0.0");
}

GBOOL EXPORT
user_logon(VOID)                     /* User Logon Message                   */
{
    // If option to display logon message is true then output message to user
    if (display_logon_msg) {
        setmbk(modmb);
        prfmsg(LOGIN);
        outprf(usrnum);
        rstmbk();
    }

    return(0);
}

GBOOL EXPORT
module_main(VOID)                   /* Main module input routine              */
{
    setmbk(modmb);
    switch (usrptr->substt) {
        case 0:
            // Check if user exists in DAT file then display menu
            store_user_record(usaptr->userid);
            prfmsg(usrptr->substt = DISPMEN);
            break;
        case DISPMEN:
            if (sameas(margv[0], "x")) {
                prfmsg(LEAVE, usaptr->userid); // User selected to exit the module
                return(0);
            }
            else if (sameas(margv[0], "\0") || sameas(margv[0], "")) {
                                               // Do nothing, will default to short menu
            }                                  
            else if (sameas(margv[0], "?")) {
                prfmsg(DISPMEN);               // Re-display the menu on ?
                break;
            }
            else if (sameas(margv[0], "c")) {
                if (!ai_handle)
                {
                    ai_handle = Chat_Create();
                    Chat_SetCallback(ai_handle, on_chunk);
                    Chat_InitializeChat(ai_handle, "gpt-4o", "You are a helpful assistant.  Your responses should only contain ASCII characters.");
                }
                usrptr->substt = CHATTING;     // Start a chat
                return 1;
            }
            else 
                prfmsg(INVSEL);                // Invalid selection message
            prfmsg(SHORTM);
            break;
        case CHATTING:
            if (margc == 1 && sameas(margv[0], "x")) {
                prfmsg(usrptr->substt = DISPMEN); // Exit to the module main menu
            }
            else if (sameas(margv[0], "\0") || sameas(margv[0], "")) {
                                                 // Do nothing on empty user input
            }               
            else {
                rstrin();
                ai_chat(input);
                return 1;
            }
            break;
    }
    outprf(usrnum);
    rstmbk();
    return 1;
}

VOID EXPORT
store_user_record(CHAR *userid)      /* Store user into DAT file              */
{
    struct moddat modd;

    dfaSetBlk(moddat);

    // If user not already in DAT file, insert them
    if (!dfaAcqEQ(&modd, userid, 0)) {
        setmem(&modd, sizeof(struct moddat), 0);
        strcpy(modd.name, userid);
        dfaInsert(&modd);
    }
    dfaRstBlk();
}

VOID EXPORT 
display_user_records(VOID)           /* List users from data file             */
{
    struct moddat modd;
    dfaSetBlk(moddat);

    INT recpos = 0;

    // Read first record in DAT file
    recpos = dfaStepLO(&modd);

    if (recpos == 1) {
        prfmsg(USRLSTS);
        prfmsg(USRLSTU, modd.name);
        while ((recpos = dfaStepNX(&modd)) != 0)
            prfmsg(USRLSTU, modd.name);        
        prfmsg(USRLSTE);
        outprf(usrnum);
    } else {
        prfmsg(NOUSERS);
        outprf(usrnum);
    }
    dfaRstBlk();
}

/*
 * If a user is deleted from the BBS, 
 * delete them from the module's database.
 */
VOID EXPORT
user_deleted(CHAR* userid)       
{
    dfaSetBlk(moddat);
    if (dfaAcqEQ(NULL, userid, 0)) 
        dfaDelete();
    dfaRstBlk();
}

VOID EXPORT
system_shutdown(VOID)                /* close MSG and DAT files for shutdown  */
{
    cleanup_mbbsai();
    clsmsg(modmb);
    dfaClose(moddat);
}

VOID EXPORT
end_mbbsai(VOID)                     /* used to help GALEXCEP.OUT analysis    */
{
    cleanup_mbbsai();
}

VOID EXPORT
on_chunk(const char* utf8)
{
    prf("%s", utf8);
    injoth();
}

INT EXPORT
show_message()
{
    btuxmn(usrnum, prfbuf);
    return(1);
}

VOID EXPORT
ai_chat(CHAR* usertext)
{
    Chat_SendAsync(ai_handle, usertext);
}

VOID EXPORT
cleanup_mbbsai(VOID)
{
    if (ai_handle)
    {
        Chat_Destroy(ai_handle);
        ai_handle = NULL;
    }
}