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
#include "AIWrapper_C.h"

// function declarations
GBOOL user_logon(VOID);                         /* Logon routine                        */
GBOOL module_main(VOID);                        /* Main module routine                  */         
VOID  user_deleted(CHAR* userid);               /* Delete account routine               */
VOID  store_user_record(CHAR* userid);          /* Store user record in DAT file        */
VOID  display_user_records(VOID);               /* Display users in DAT file            */
VOID  system_shutdown(VOID);                    /* Shutdown routine                     */    
VOID  end_mbbsai(VOID);                         /* empty fcn to show EOF for debug      */
VOID  cleanup_mbbsai(VOID);                     /* Dispose of open handle if needed     */
VOID  ai_chat(INT chan, CHAR* usertext);        /* Call the AI chat function            */
VOID  ai_image(INT chan, CHAR* usertext);       /* Call the AI image function           */
VOID  on_msg_chunk(INT cid,                     /* output chunked response data         */
                   const char* uid, 
                   const char* txt, 
                   ULONG rt, 
                   ULONG st, 
                   int fin, 
                   int err);        
VOID  on_img_recv(INT cid,                      /* output image data                    */
	              const char* uid,
	              const char* imgANSI,
	              ULONG rt,
	              ULONG st,
	              int err,
	              const char* errMsg);


// module definition
INT usrstt;                          /* User state in this module            */
struct module MBBSAI={               /* module interface block               */
     "",                             /* name used to refer to this module    */
     user_logon,                     /* user logon supplemental routine      */
     module_main,                    /* input routine if selected            */
     NULL,                           /* status-input routine if selected     */
     NULL,                           /* "injoth" routine for this module     */
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

static AI_HANDLE ai_handle;          /* Handle for the AI chat                */

// TODO: track token usage
struct moddat {                      /* Btrieve database file structure      */
    CHAR name[UIDSIZ];               /* userid                               */
	ULONG htsend[12];                /* 12 month history of tokens sent      */
	ULONG htrecv[12];                /* 12 month history of tokens received  */
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
    return 0;
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
            else if (sameas(margv[0], "c")) {   // Start a chat
                if (!ai_handle)
                {
                    //TODO: Make model and prompt configurable by sysop
                    ai_handle = AI_Create("gpt-4o", "dall-e-3",
                        "You are a helpful AI assistant named BBS Bot. Your responses should only contain ASCII characters or ANSI escape sequences.");
                    Chat_SetCallback(ai_handle, on_msg_chunk);
                    Image_SetCallback(ai_handle, on_img_recv);
                }
				AI_StartSession(ai_handle, usrnum, usaptr->userid);
                usrptr->substt = CHATTING;     
                prf(usaptr->userid);
                prf(": ");
				outprf(usrnum);
                rstmbk();
                return 1;
            }
            else 
                prfmsg(INVSEL);                // Invalid selection message
            prfmsg(SHORTM);
            break;
        case CHATTING:
            if (margc == 1 && sameas(margv[0], "x")) {
				AI_EndSession(ai_handle, usrnum);
                prfmsg(usrptr->substt = DISPMEN); // Exit to the module main menu
            }
            else if (sameas(margv[0], "\0") || sameas(margv[0], "")) {
                                                 // Do nothing on empty user input
            }  
            else if (sameas(margv[0], "clear")) { // Reset chat history for the session
                Chat_ClearHistory(ai_handle, usrnum);
				prfmsg(CLEARED);
                prf("%s", usaptr->userid);
            }
            else {
                rstrin();
				prfmsg(BOTNAME, "BBS Bot:\n");  //TODO: Make bot name sysop configurable
				outprf(usrnum);
                rstmbk();
                ai_chat(usrnum, input);
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
        for (int i = 0; i < 12; i++) {
            modd.htsend[i] = 0;
			modd.htrecv[i] = 0;
        }
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
on_msg_chunk(INT cid, const char* uid, const char* txt, ULONG rt, ULONG st, int fin, int err)
{
    prf("%s", txt);
    if (fin)
        prf("\r\n%s: ", uid);
	outprf(cid);
}

VOID EXPORT
on_img_recv(INT cid, const char* uid, const char* imgANSI, ULONG rt, ULONG st, int err, const char* errMsg)
{
	if (err)
		prf("Error: %s", errMsg);
	else
		prf("%s\r\n%s", imgANSI, uid);
	outprf(cid);
}

VOID EXPORT
ai_chat(INT chan, CHAR* usertext)
{
    Chat_SendAsync(ai_handle, chan, usertext);
}

VOID EXPORT
ai_image(INT chan, CHAR* usertext)
{
	Image_PromptAsync(ai_handle, chan, usertext);
}

VOID EXPORT
cleanup_mbbsai(VOID)
{
    if (ai_handle)
    {
        AI_Destroy(ai_handle);
        ai_handle = NULL;
    }
}