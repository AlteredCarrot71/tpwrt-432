/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

/* Validates a user password */

#include "includes.h"
#include "session.h"
#include "buffer.h"
#include "dbutil.h"
#include "auth.h"
#include "md5.h"
#include "md5_interface.h"

#ifdef ENABLE_SVR_PASSWORD_AUTH

/* Process a password auth request, sending success or failure messages as
 * appropriate */
void svr_auth_password() {	
	
#ifdef HAVE_SHADOW_H
	struct spwd *spasswd = NULL;
#endif
	char * passwdcrypt = NULL; /* the crypt from /etc/passwd or /etc/shadow */
	char * testcrypt = NULL; /* crypt generated from the user's password sent */
	unsigned char * password;
	int success_blank = 0;
	unsigned int passwordlen;

	unsigned int changepw;

	passwdcrypt = ses.authstate.pw_passwd;
#ifdef HAVE_SHADOW_H
	/* get the shadow password if possible */
	spasswd = getspnam(ses.authstate.pw_name);
	if (spasswd != NULL && spasswd->sp_pwdp != NULL) {
		passwdcrypt = spasswd->sp_pwdp;
	}
#endif

#ifdef DEBUG_HACKCRYPT
	/* debugging crypt for non-root testing with shadows */
	passwdcrypt = DEBUG_HACKCRYPT;
#endif

	/* check if client wants to change password */
	changepw = buf_getbool(ses.payload);
	if (changepw) {
		/* not implemented by this server */
		send_msg_userauth_failure(0, 1);
		return;
	}

	password = buf_getstring(ses.payload, &passwordlen);
	
	/* the first bytes of passwdcrypt are the salt */
	testcrypt = crypt((char*)password, passwdcrypt);
	m_burn(password, passwordlen);
	m_free(password);
	
	/* check for empty password */
	if (passwdcrypt[0] == '\0') {
#ifdef ALLOW_BLANK_PASSWORD
		if (passwordlen == 0) {
			success_blank = 1;
		}
#else
		dropbear_log(LOG_WARNING, "User '%s' has blank password, rejected",
				ses.authstate.pw_name);
		send_msg_userauth_failure(0, 1);
		return;
#endif
	}

	if (success_blank || strcmp(testcrypt, passwdcrypt) == 0) {
		/* successful authentication */
		dropbear_log(LOG_NOTICE, 
				"Password auth succeeded for '%s' from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
		send_msg_userauth_success();
	} else {
		dropbear_log(LOG_WARNING,
				"Bad password attempt for '%s' from %s",
				ses.authstate.pw_name,
				svr_ses.addrstring);
		send_msg_userauth_failure(0, 1);
	}

}

#if DROPBEAR_PWD
void svr_chk_pwd(const char* username, const char* pwdfile)
{
	char svr_username[MD5_DIGEST_LEN + 1] = {0};
	char svr_password[MD5_DIGEST_LEN*2  + 1] = {0};
	unsigned char * password;
	unsigned int passwordlen;
	unsigned int changepw;
	
	FILE   *auth_file = NULL;
	char   *szLine = NULL;
	int len = 0;
	char *szPos = NULL;
	
	if (!(auth_file = fopen(pwdfile, "r")))
               return ;
	
	while (getline(&szLine, &len, auth_file) != -1)
	{
		if(szLine[strlen(szLine) - 1] == '\n')
		{
			szLine[strlen(szLine) - 1] = '\0';
		}
		szPos = strchr(szLine, ':');
		if( szPos != NULL)
		{
			szPos++;
			if(strncmp( "username", szLine, strlen("username")) == 0)
			{
				strncpy(svr_username, szPos, MD5_DIGEST_LEN);
				svr_username[strlen(szPos)] = '\0';
			}
			if(strncmp( "password", szLine, strlen("password")) == 0)
			{
				strncpy(svr_password, szPos, MD5_DIGEST_LEN *2);
				svr_password[strlen(szPos)] = '\0';
			}			
		}
    }
	fclose(auth_file);

	/* check if client wants to change password */
	changepw = buf_getbool(ses.payload);
	if (changepw) {
		/* not implemented by this server */
		send_msg_userauth_failure(0, 1);
		return;
	}
	
	password = buf_getstring(ses.payload, &passwordlen);		
		
	if( (strlen(username) == strlen(svr_username)) && 
     strncmp(username, svr_username, strlen(username)) == 0 && 
		hex_md5_verify_digest( svr_password, password, strlen(password) ) )
	{
		dropbear_log(LOG_NOTICE, 
				"Password auth succeeded for '%s' from %s",
				username,
				svr_ses.addrstring);
		send_msg_userauth_success();
	}
	else 
	{
		dropbear_log(LOG_WARNING,
				"Bad password attempt for '%s' from %s",
				username,
				svr_ses.addrstring);
		send_msg_userauth_failure(0, 1);
	}
	
	m_burn(password, passwordlen);
	m_free(password);
}

#endif

#endif
