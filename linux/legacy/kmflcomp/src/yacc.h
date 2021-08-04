/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_YACC_H_INCLUDED
# define YY_YY_YACC_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_ANY = 258,
    TOK_ANSI = 259,
    TOK_AUTHOR = 260,
    TOK_BEEP = 261,
    TOK_BITMAP = 262,
    TOK_BRKT = 263,
    TOK_CALL = 264,
    TOK_CAPSFREE = 265,
    TOK_CAPSOFF = 266,
    TOK_CAPSON = 267,
    TOK_COMMA = 268,
    TOK_CONTEXT = 269,
    TOK_COPYRIGHT = 270,
    TOK_CHAR = 271,
    TOK_DEADKEY = 272,
    TOK_DOLLAR = 273,
    TOK_ERROR = 274,
    TOK_ETHNOLOGUE = 275,
    TOK_GROUP = 276,
    TOK_GT = 277,
    TOK_HOTKEY = 278,
    TOK_INDEX = 279,
    TOK_LANGUAGE = 280,
    TOK_LAYOUT = 281,
    TOK_KEYBOARDVERSION = 282,
    TOK_MATCH = 283,
    TOK_MESSAGE = 284,
    TOK_MNEMONIC = 285,
    TOK_NOMATCH = 286,
    TOK_NAME = 287,
    TOK_NOTANY = 288,
    TOK_NUL = 289,
    TOK_NUMBER = 290,
    TOK_NL = 291,
    TOK_OUTS = 292,
    TOK_RAWKEY = 293,
    TOK_PLUS = 294,
    TOK_QM = 295,
    TOK_RTN = 296,
    TOK_SB = 297,
    TOK_SHIFT = 298,
    TOK_STORE = 299,
    TOK_STOREINSTORE = 300,
    TOK_SWITCH = 301,
    TOK_TARGETS = 302,
    TOK_UNICODE = 303,
    TOK_USE = 304,
    TOK_USINGKEYS = 305,
    TOK_UTF = 306,
    TOK_VERSION = 307,
    TOK_VISUALKEYBOARD = 308,
    TOK_XKEYSYM = 309
  };
#endif
/* Tokens.  */
#define TOK_ANY 258
#define TOK_ANSI 259
#define TOK_AUTHOR 260
#define TOK_BEEP 261
#define TOK_BITMAP 262
#define TOK_BRKT 263
#define TOK_CALL 264
#define TOK_CAPSFREE 265
#define TOK_CAPSOFF 266
#define TOK_CAPSON 267
#define TOK_COMMA 268
#define TOK_CONTEXT 269
#define TOK_COPYRIGHT 270
#define TOK_CHAR 271
#define TOK_DEADKEY 272
#define TOK_DOLLAR 273
#define TOK_ERROR 274
#define TOK_ETHNOLOGUE 275
#define TOK_GROUP 276
#define TOK_GT 277
#define TOK_HOTKEY 278
#define TOK_INDEX 279
#define TOK_LANGUAGE 280
#define TOK_LAYOUT 281
#define TOK_KEYBOARDVERSION 282
#define TOK_MATCH 283
#define TOK_MESSAGE 284
#define TOK_MNEMONIC 285
#define TOK_NOMATCH 286
#define TOK_NAME 287
#define TOK_NOTANY 288
#define TOK_NUL 289
#define TOK_NUMBER 290
#define TOK_NL 291
#define TOK_OUTS 292
#define TOK_RAWKEY 293
#define TOK_PLUS 294
#define TOK_QM 295
#define TOK_RTN 296
#define TOK_SB 297
#define TOK_SHIFT 298
#define TOK_STORE 299
#define TOK_STOREINSTORE 300
#define TOK_SWITCH 301
#define TOK_TARGETS 302
#define TOK_UNICODE 303
#define TOK_USE 304
#define TOK_USINGKEYS 305
#define TOK_UTF 306
#define TOK_VERSION 307
#define TOK_VISUALKEYBOARD 308
#define TOK_XKEYSYM 309

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 25 "../../kmflcomp/src/yacc.y" /* yacc.c:1909  */

	int simple;
	ITEM number;
	char *string;
	ITEM *items;
	RULE *rule;
	GROUP *group;
	

#line 172 "yacc.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_YACC_H_INCLUDED  */
