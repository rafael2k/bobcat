/*
***************************************************************************
* This file comprises part of PDCurses. PDCurses is Public Domain software.
* You may use this code for whatever purposes you desire. This software
* is provided AS IS with NO WARRANTY whatsoever.
* Should this software be used in another application, an acknowledgement
* that PDCurses code is used would be appreciated, but is not mandatory.
*
* Any changes which you make to this software which may improve or enhance
* it, should be forwarded to the current maintainer for the benefit of 
* other users.
*
* The only restriction placed on this code is that no distribution of
* modified PDCurses code be made under the PDCurses name, by anyone
* other than the current maintainer.
* 
* See the file maintain.er for details of the current maintainer.
***************************************************************************
*/
#define	CURSES_LIBRARY	1
#include <curses.h>
#ifdef UNIX
#include <defs.h>
#include <term.h>
#endif

#ifdef PDCDEBUG
char *rcsid_PDC_setkeys  = "$Id$";
#endif




/*man-start*********************************************************************

  PDC_setup_keys()	- Setup function key translations.

  PDCurses Description:
 	This is a private PDCurses routine.

 	Sets up the array of key sequences and their associated curses key number
 	for use by the getch() function when determining if a sequence of
 	characters is a key escape sequence.


  PDCurses Return Value:
 	This function returns OK on success and ERR on error.

  PDCurses Errors:
 	No errors are defined for this function.

  Portability:
 	PDCurses	int PDC_setup_keys( char *keyptr, int keynum );

**man-end**********************************************************************/


#ifdef UNIX
static void add_key(char *keyptr,int keynum)
{
	if (keyptr != NULL)
		{
#ifdef PDCDEBUG
		if (trace_on) PDC_debug("add_key() keyval %s keynum %d\n",keyptr,keynum);
#endif
		_cursvar.key_seq[_cursvar.number_keys] = keyptr;
		_cursvar.key_num[_cursvar.number_keys++] = keynum;
		}
	return;
}
int PDC_setup_keys()
{
#ifdef PDCDEBUG
	if (trace_on) PDC_debug("PDC_setup_keys() - called\n");
#endif
	_cursvar.number_keys = 0;
	add_key(key_a1       ,KEY_A1       );
	add_key(key_a3       ,KEY_A3       );
	add_key(key_b2       ,KEY_B2       );
	add_key(key_backspace,KEY_BACKSPACE);
	add_key(key_beg      ,KEY_BEG      );
	add_key(key_btab     ,KEY_BTAB     );
	add_key(key_c1       ,KEY_C1       );
	add_key(key_c3       ,KEY_C3       );
	add_key(key_cancel   ,KEY_CANCEL   );
	add_key(key_catab    ,KEY_CATAB    );
	add_key(key_clear    ,KEY_CLEAR    );
	add_key(key_close    ,KEY_CLOSE    );
	add_key(key_command  ,KEY_COMMAND  );
	add_key(key_copy     ,KEY_COPY     );
	add_key(key_create   ,KEY_CREATE   );
	add_key(key_ctab     ,KEY_CTAB     );
	add_key(key_dc       ,KEY_DC       );
	add_key(key_dl       ,KEY_DL       );
	add_key(key_down     ,KEY_DOWN     );
	add_key(key_eic      ,KEY_EIC      );
	add_key(key_end      ,KEY_END      );
	add_key(key_enter    ,KEY_ENTER    );
	add_key(key_eol      ,KEY_EOL      );
	add_key(key_eos      ,KEY_EOS      );
	add_key(key_exit     ,KEY_EXIT     );
	add_key(key_f0       ,KEY_F(0 )    );
	add_key(key_f1       ,KEY_F(1 )    );
	add_key(key_f2       ,KEY_F(2 )    );
	add_key(key_f3       ,KEY_F(3 )    );
	add_key(key_f4       ,KEY_F(4 )    );
	add_key(key_f5       ,KEY_F(5 )    );
	add_key(key_f6       ,KEY_F(6 )    );
	add_key(key_f7       ,KEY_F(7 )    );
	add_key(key_f8       ,KEY_F(8 )    );
	add_key(key_f9       ,KEY_F(9 )    );
	add_key(key_f10      ,KEY_F(10)    );
	add_key(key_f11      ,KEY_F(11)    );
	add_key(key_f12      ,KEY_F(12)    );
	add_key(key_f13      ,KEY_F(13)    );
	add_key(key_f14      ,KEY_F(14)    );
	add_key(key_f15      ,KEY_F(15)    );
	add_key(key_f16      ,KEY_F(16)    );
	add_key(key_f17      ,KEY_F(17)    );
	add_key(key_f18      ,KEY_F(18)    );
	add_key(key_f19      ,KEY_F(19)    );
	add_key(key_f20      ,KEY_F(20)    );
	add_key(key_f21      ,KEY_F(21)    );
	add_key(key_f22      ,KEY_F(22)    );
	add_key(key_f23      ,KEY_F(23)    );
	add_key(key_f24      ,KEY_F(24)    );
	add_key(key_f25      ,KEY_F(25)    );
	add_key(key_f26      ,KEY_F(26)    );
	add_key(key_f27      ,KEY_F(27)    );
	add_key(key_f28      ,KEY_F(28)    );
	add_key(key_f29      ,KEY_F(29)    );
	add_key(key_f30      ,KEY_F(30)    );
	add_key(key_f31      ,KEY_F(31)    );
	add_key(key_f32      ,KEY_F(32)    );
	add_key(key_f33      ,KEY_F(33)    );
	add_key(key_f34      ,KEY_F(34)    );
	add_key(key_f35      ,KEY_F(35)    );
	add_key(key_f36      ,KEY_F(36)    );
	add_key(key_f37      ,KEY_F(37)    );
	add_key(key_f38      ,KEY_F(38)    );
	add_key(key_f39      ,KEY_F(39)    );
	add_key(key_f40      ,KEY_F(40)    );
	add_key(key_f41      ,KEY_F(41)    );
	add_key(key_f42      ,KEY_F(42)    );
	add_key(key_f43      ,KEY_F(43)    );
	add_key(key_f44      ,KEY_F(44)    );
	add_key(key_f45      ,KEY_F(45)    );
	add_key(key_f46      ,KEY_F(46)    );
	add_key(key_f47      ,KEY_F(47)    );
	add_key(key_f48      ,KEY_F(48)    );
	add_key(key_f49      ,KEY_F(49)    );
	add_key(key_f50      ,KEY_F(50)    );
	add_key(key_f51      ,KEY_F(51)    );
	add_key(key_f52      ,KEY_F(52)    );
	add_key(key_f53      ,KEY_F(53)    );
	add_key(key_f54      ,KEY_F(54)    );
	add_key(key_f55      ,KEY_F(55)    );
	add_key(key_f56      ,KEY_F(56)    );
	add_key(key_f57      ,KEY_F(57)    );
	add_key(key_f58      ,KEY_F(58)    );
	add_key(key_f59      ,KEY_F(59)    );
	add_key(key_f60      ,KEY_F(60)    );
	add_key(key_f61      ,KEY_F(61)    );
	add_key(key_f62      ,KEY_F(62)    );
	add_key(key_f63      ,KEY_F(63)    );
	add_key(key_find     ,KEY_FIND     );
	add_key(key_help     ,KEY_HELP     );
	add_key(key_home     ,KEY_HOME     );
	add_key(key_ic       ,KEY_IC       );
	add_key(key_il       ,KEY_IL       );
	add_key(key_left     ,KEY_LEFT     );
	add_key(key_ll       ,KEY_LL       );
	add_key(key_mark     ,KEY_MARK     );
	add_key(key_message  ,KEY_MESSAGE  );
	add_key(key_move     ,KEY_MOVE     );
	add_key(key_next     ,KEY_NEXT     );
	add_key(key_npage    ,KEY_NPAGE    );
	add_key(key_open     ,KEY_OPEN     );
	add_key(key_options  ,KEY_OPTIONS  );
	add_key(key_ppage    ,KEY_PPAGE    );
	add_key(key_previous ,KEY_PREVIOUS );
	add_key(key_print    ,KEY_PRINT    );
	add_key(key_redo     ,KEY_REDO     );
	add_key(key_reference,KEY_REFERENCE);
	add_key(key_refresh  ,KEY_REFRESH  );
	add_key(key_replace  ,KEY_REPLACE  );
	add_key(key_restart  ,KEY_RESTART  );
	add_key(key_resume   ,KEY_RESUME   );
	add_key(key_right    ,KEY_RIGHT    );
	add_key(key_save     ,KEY_SAVE     );
	add_key(key_sbeg     ,KEY_SBEG     );
	add_key(key_scancel  ,KEY_SCANCEL  );
	add_key(key_scommand ,KEY_SCOMMAND );
	add_key(key_scopy    ,KEY_SCOPY    );
	add_key(key_screate  ,KEY_SCREATE  );
	add_key(key_sdc      ,KEY_SDC      );
	add_key(key_sdl      ,KEY_SDL      );
	add_key(key_select   ,KEY_SELECT   );
	add_key(key_send     ,KEY_SEND     );
	add_key(key_seol     ,KEY_SEOL     );
	add_key(key_sexit    ,KEY_SEXIT    );
	add_key(key_sf       ,KEY_SF       );
	add_key(key_sfind    ,KEY_SFIND    );
	add_key(key_shelp    ,KEY_SHELP    );
	add_key(key_shome    ,KEY_SHOME    );
	add_key(key_sic      ,KEY_SIC      );
	add_key(key_sleft    ,KEY_SLEFT    );
	add_key(key_smessage ,KEY_SMESSAGE );
	add_key(key_smove    ,KEY_SMOVE    );
	add_key(key_snext    ,KEY_SNEXT    );
	add_key(key_soptions ,KEY_SOPTIONS );
	add_key(key_sprevious,KEY_SPREVIOUS);
	add_key(key_sprint   ,KEY_SPRINT   );
	add_key(key_sr       ,KEY_SR       );
	add_key(key_sredo    ,KEY_SREDO    );
	add_key(key_sreplace ,KEY_SREPLACE );
	add_key(key_sright   ,KEY_SRIGHT   );
	add_key(key_srsume   ,KEY_SRSUME   );
	add_key(key_ssave    ,KEY_SSAVE    );
	add_key(key_ssuspend ,KEY_SSUSPEND );
	add_key(key_stab     ,KEY_STAB     );
	add_key(key_sundo    ,KEY_SUNDO    );
	add_key(key_suspend  ,KEY_SUSPEND  );
	add_key(key_undo     ,KEY_UNDO     );
	add_key(key_up       ,KEY_UP       );
}
#endif
