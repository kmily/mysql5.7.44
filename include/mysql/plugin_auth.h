#ifndef MYSQL_PLUGIN_AUTH_INCLUDED
/* Copyright (c) 2010, 2023, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file

  Authentication Plugin API.

  This file defines the API for server authentication plugins.
*/

#define MYSQL_PLUGIN_AUTH_INCLUDED

#include <mysql/plugin.h>

#define MYSQL_AUTHENTICATION_INTERFACE_VERSION 0x0101

#include "plugin_auth_common.h"

/* defines for MYSQL_SERVER_AUTH_INFO.password_used */

#define PASSWORD_USED_NO         0
#define PASSWORD_USED_YES        1
#define PASSWORD_USED_NO_MENTION 2

/* Authentication flags */

#define AUTH_FLAG_PRIVILEGED_USER_FOR_PASSWORD_CHANGE (1L << 0)
#define AUTH_FLAG_USES_INTERNAL_STORAGE               (1L << 1)

/**
  Provides server plugin access to authentication information
*/
typedef struct st_mysql_server_auth_info
{
  /**
    User name as sent by the client and shown in USER().
    NULL if the client packet with the user name was not received yet.
  */
  char *user_name;

  /**
    Length of user_name
  */
  unsigned int user_name_length;

  /**
    A corresponding column value from the mysql.user table for the
    matching account name
  */
  const char *auth_string;

  /**
    Length of auth_string
  */
  unsigned long auth_string_length;

  /**
    Matching account name as found in the mysql.user table.
    A plugin can override it with another name that will be
    used by MySQL for authorization, and shown in CURRENT_USER()
  */
  char authenticated_as[MYSQL_USERNAME_LENGTH+1]; 


  /**
    The unique user name that was used by the plugin to authenticate.
    Plugins should put null-terminated UTF-8 here.
    Available through the @@EXTERNAL_USER variable.
  */  
  char external_user[512];

  /**
    This only affects the "Authentication failed. Password used: %s"
    error message. has the following values : 
    0 : %s will be NO.
    1 : %s will be YES.
    2 : there will be no %s.
    Set it as appropriate or ignore at will.
  */
  int  password_used;

  /**
    Set to the name of the connected client host, if it can be resolved, 
    or to its IP address otherwise.
  */
  const char *host_or_ip;

  /**
    Length of host_or_ip
  */
  unsigned int host_or_ip_length;

} MYSQL_SERVER_AUTH_INFO;

/**
  Server authentication plugin descriptor
*/
struct st_mysql_auth
{
  int interface_version;                        /** version plugin uses */
  /**
    A plugin that a client must use for authentication with this server
    plugin. Can be NULL to mean "any plugin".
  */
  const char *client_auth_plugin;
  /**
    Function provided by the plugin which should perform authentication (using
    the vio functions if necessary) and return 0 if successful. The plugin can
    also fill the info.authenticated_as field if a different username should be
    used for authorization.
  */
  int (*authenticate_user)(MYSQL_PLUGIN_VIO *vio, MYSQL_SERVER_AUTH_INFO *info);
  /**
    New plugin API to generate password digest out of authentication string.
    This function will first invoke a service to check for validity of the
    password based on the policies defined and then generate encrypted hash

    @param[OUT]   outbuf      A buffer provided by server which will hold the
                              authentication string generated by plugin.
    @param[INOUT] outbuflen   Length of server provided buffer as IN param and
                              length of plugin generated string as OUT param.
    @param[IN]    inbuf       auth string provided by user.
    @param[IN]    inbuflen    auth string length.

    @retval  0 OK
             1 ERROR

  */
  int (*generate_authentication_string)(char *outbuf,
      unsigned int *outbuflen, const char *inbuf, unsigned int inbuflen);
  /**
    Plugin API to validate password digest.

    @param[IN]    inbuf     hash string to be validated.
    @param[IN]    buflen    hash string length.

    @retval  0 OK
             1 ERROR

  */
  int (*validate_authentication_string)(char* const inbuf, unsigned int buflen);
  /**
   Plugin API to convert scrambled password to binary form
   based on scramble type.

   @param[IN]    password          The password hash containing the salt.
   @param[IN]    password_len      The length of the password hash.
   @param[INOUT] salt              Used as password hash based on the
                                   authentication plugin.
   @param[INOUT] salt_len          The length of salt.

   @retval  0 OK
            1 ERROR

   */
  int (*set_salt)(const char *password, unsigned int password_len,
                  unsigned char* salt, unsigned char *salt_len);
  /**
    Authentication plugin capabilities
  */
  const unsigned long authentication_flags;
};
#endif

