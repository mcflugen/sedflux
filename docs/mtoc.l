/*
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby 
 * granted. No representations are made about the suitability of this software 
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 *
 * Ian Martins (ianxm@jhu.edu)
 */

	int in_function = 0;			/* in function */
	int inCommentLine = 0;			/* in comment line */
	int comment_caller;
	void closeCommentBlock();
ID	[a-zA-Z][0-9a-zA-Z_]*
WS	[ \t]*

%s func funcDef paramList commentBlock
%%

<func>"%%"	{				/* end last function */
		  comment_caller = YY_START;
		  BEGIN(commentBlock);
		  //fprintf(yyout,"}\n/**");
		  fprintf(yyout,"\n\n/**");
		  inCommentLine = 1;		/* start of comment */
		}
<func>[[:graph:][:space:]]* { }
<commentBlock>"%%"		{
		  fprintf(yyout,"*/\n/**");	/* close old comment, start new one */
		  inCommentLine = 1;		/* start of comment */
		}
"%%"		{				/* first token */
		  comment_caller = YY_START;
		  BEGIN(commentBlock);
		  fprintf(yyout,"/**");
		  inCommentLine = 1;		/* start of comment */
		}
<commentBlock>"%"	{
		  if (inCommentLine)		/* just a symbol in a comment */
		    ECHO;
		  else 				/* part of current comment block */
		  {
		    fprintf(yyout,"*");
		    inCommentLine = 1;
		  }
		}
"%"		{				/* start comment block */
                  if ( YY_START!=func )
                  {
		     fprintf(yyout,"/*");
		     comment_caller = YY_START;
		     BEGIN(commentBlock);
		     inCommentLine = 1;		/* start of comment */
                  }
		}

\n		{ ECHO; inCommentLine = 0; }
^[ \t]*function	{
		  closeCommentBlock(); 
		  fprintf(yyout,"void\n");
		  BEGIN(funcDef);
                  in_function = 1;
		}
^[ \t]*function.*\[.*\].*=	{
		  closeCommentBlock(); 
		  fprintf(yyout,"rets\n");
		  BEGIN(funcDef);
                  in_function = 1;
		}
^[ \t]*function.*=	{
		  closeCommentBlock(); 
		  fprintf(yyout,"ret\n");
		  BEGIN(funcDef);
                  in_function = 1;
		}
<funcDef>[[:space:]] {}
<funcDef>\(	{
		  ECHO;
		  BEGIN(paramList);
		}
<paramList>\)	{
		  ECHO;
		  //fprintf(yyout,"\n{\n");
		  fprintf(yyout,";\n");
		  BEGIN(func);
		}
<paramList>filename|file|file_name {
                  fprintf(yyout,"char ");
		  ECHO;
                }
<paramList>fid {
                  fprintf(yyout,"int ");
		  ECHO;
                }
<paramList>varargin {
                  fprintf(yyout,"cell ");
		  ECHO;
                }
<paramList>[a-zA-Z][0-9a-zA-Z_]*	{
		  fprintf(yyout,"type ");
		  ECHO;
		}
<commentBlock>[0-9a-zA-Z]+	{
		  closeCommentBlock();
		  ECHO;
		}

[0-9a-zA-Z_.]+	{
                  if ( YY_START!=func )
		     ECHO;
		}
<<EOF>>		{
		  //fprintf(yyout,"\n}\n");
		  return;
		}

%%

void closeCommentBlock()			/* end comment block */
{
   if ( YY_START==commentBlock && !inCommentLine )
//  if (!inCommentLine)
  {
    fprintf(yyout,"*/\n");
    BEGIN(comment_caller);
  }
}

int
main( int argc, char **argv )
{
   if ( argc > 1 )
      yyin = fopen( argv[1], "r" );
   else
      yyin = stdin;

   if ( yyin )
   {
      if ( argc > 2 )
         yyout = fopen( argv[2], "w" );
      else
         yyout = stdout;
      yylex();
   }
   return 0;
}

