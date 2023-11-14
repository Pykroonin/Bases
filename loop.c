/*
 Created by roberto on 30/4/21.
 IT will not process properly UTF8
 export LC_ALL=C

 Function in this file assume a windows setup as:

 +-------------------------------+ <-- main_win
 |+-----------------------------+|
 ||          win_menu           ||
 |+-----------------------------+|
 |+--------++-------------------+|
 ||        ||                   ||
 ||        ||                   ||
 ||win_form||   win_out         ||
 ||        ||                   ||
 ||        ||                   ||
 |+--------++-------------------+|
 |+-----------------------------+|
 ||          win_messages       ||
 |+-----------------------------+|
 +-------------------------------+
*/
#include "loop.h"
#include <string.h>
#include <stdio.h>

void loop(_Windows *windows, _Menus *menus,
          _Forms *forms, _Panels *panels) {
    /** get keys pressed by user and process it.
     * - If left/right arrow is pressed  move to
     * the next/previous menu item
     * - If up/down key is pressed check on focus
     * variable if focus == FOCUS_LEFT move to the
     * previous/next form item in the left window
     * (aka form_window). Otherwise do the same
     * in the win_out.
     * - If enter is pressed the action depend on the
     * item selected in the menu bar and the value of
     * "focus":
     *  a) focus = FOCUS_LEFT
     *     quit -> quit the application
     *     search -> execute  results_search
     *               and fill win_out
     *     bpass ->  execute  bpas_search
     *               and fill win_out
     *  b) focus == FOCUS_RIGHT
     *     show the highlighted line in
     *     win_out in the win_message window
     *  - tab key toggle focus between
     *  win_form and win_out windows
     */
    int focus = FOCUS_LEFT; /* focus is in win_form/win_out window */
    int ch=0; /* typed character */
    bool enterKey = FALSE; /* has enter been presswed ? */
    char buffer[128]; /* auxiliary buffer to compose messages */
    ITEM *auxItem = (ITEM *) NULL; /* item selected in the menu */
    int choice = -1; /* index of the item selected in menu*/
    MENU *menu = NULL; /* pointer to menu in menu_win*/
    WINDOW *menu_win= NULL; /* pointer to menu_win */
    WINDOW *out_win= NULL; /* pointer to out_win */
    WINDOW *msg_win= NULL; /* pointer to msg_win */
    char * tmpStr1= NULL; /* used to read values typed in forms */
    char * tmpStr2= NULL; /* used to read values typed in forms */
    char * tmpStr3= NULL; /* used to read values typed in forms */
    int n_out_choices=0; /* number of printed lines in win_out window */
    int out_highlight = 0; /* line highlighted in win_out window */
    int rows_out_window = 0; /* size of win_out window */
    int i = 0; /* dummy variable for loops */
    int page=0;
    int des=0;
    int max_pages=0;
    int rows_last_page=1;
    int max_rows;

    (void) curs_set(1); /* show cursor */
    menu = menus->menu;
    menu_win = windows->menu_win;
    out_win = windows->out_win;
    msg_win = windows->msg_win;
    rows_out_window = windows->terminal_nrows - 2 * windows->height_menu_win - 1;
   /* windows->rows_out_win=7;esta linea es pa probar que va lo de las paginas*/
    max_rows=windows->rows_out_win-2;
    while ((bool) TRUE) {

        ch = getch(); /* get char typed by user */
        if ((bool)DEBUG) {
            /*(void)snprintf(buffer, 128, "key pressed %d %c (%d)",  ch, ch, item_index(auxItem)); */
            (void)snprintf(buffer, 128, "hilighted:%d, max_pages:%d, page:%d, n_out_choices:%d, max_rows:%d, max_rowlastpage:%d", out_highlight,max_pages, page, n_out_choices, max_rows, rows_last_page); 
            write_msg(msg_win, buffer, -1, -1, windows->msg_title);
        }
        switch (ch) {
            case KEY_LEFT:
            case 0x3C: /* < */
                focus = FOCUS_LEFT;
                /* auxiliary function provided by ncurses see
                 * https://docs.oracle.com/cd/E88353_01/html/E37849/menu-driver-3curses.html
                 */
                (void) menu_driver(menu, REQ_LEFT_ITEM);
                /* refresh window menu_win */
                (void) wrefresh(menu_win);
                /* get selected item from menu */
                auxItem = current_item(menu);
                /* draw the corresponding form in win_form */
                if (item_index(auxItem) == SEARCH)
                    (void) top_panel(panels->search_panel);
                else if (item_index(auxItem) == BPASS)
                    (void) top_panel(panels->bpass_panel);
                /* refresh window using the panel system.
                 * we need panels because search and bpass
                 * are attached to two ovelapping windows
                 * */
                (void) update_panels();
                (void) doupdate();
                break;
            case KEY_RIGHT:
            case 0x3E: /* > */
                focus = FOCUS_LEFT;
                (void) menu_driver(menu, REQ_RIGHT_ITEM);
                (void) wrefresh(menu_win);
                auxItem = current_item(menu);
                if (item_index(auxItem) == SEARCH)
                    (void) top_panel(panels->search_panel);
                else if (item_index(auxItem) == BPASS)
                    (void) top_panel(panels->bpass_panel);
                (void) update_panels();
                (void) doupdate();
                break;
            case KEY_UP:
            case 0x2B: /* + */
                /* form_driver is the equivalent to menu_driver for forms */
                if (item_index(auxItem) == SEARCH && focus == FOCUS_LEFT) {
                    (void) form_driver(forms->search_form, REQ_PREV_FIELD);
                    (void) form_driver(forms->search_form, REQ_END_LINE);
                    (void) wrefresh(windows->form_search_win);
                } else if (item_index(auxItem) == BPASS && focus == FOCUS_LEFT) {
                    (void) form_driver(forms->bpass_form, REQ_PREV_FIELD);
                    (void) form_driver(forms->bpass_form, REQ_END_LINE);
                    (void) wrefresh(windows->form_bpass_win);
                }  else if (focus == FOCUS_RIGHT){
                    out_highlight = MAX(out_highlight - 1, 0+des);
                    print_out(out_win, menus->out_win_choices,1+des,MIN(max_rows, n_out_choices)+des,
                                out_highlight, windows->out_title);
                }
                break;
            case KEY_DOWN:
            case 0x2D: /* - */
                if (item_index(auxItem) == SEARCH && focus == FOCUS_LEFT) {
                    (void) form_driver(forms->search_form, REQ_NEXT_FIELD);
                    (void) form_driver(forms->search_form, REQ_END_LINE);
                    (void) wrefresh(windows->form_search_win);
                } else if (item_index(auxItem) == BPASS && focus == FOCUS_LEFT) {
                    (void) form_driver(forms->bpass_form, REQ_NEXT_FIELD);
                    (void) form_driver(forms->bpass_form, REQ_END_LINE);
                    (void) wrefresh(windows->form_bpass_win);
                } else if (focus == FOCUS_RIGHT){
                    if(page==max_pages){
                        out_highlight = MIN(out_highlight + 1, rows_last_page+des-1);
                    }
                    else{
                        out_highlight = MIN(out_highlight + 1, MIN(max_rows, n_out_choices)-1+des);
                    }
                    print_out(out_win, menus->out_win_choices,1+des,MIN(max_rows, n_out_choices) +des,
                                out_highlight, windows->out_title);
                }
                break;
            case KEY_STAB: /* tab key */
            case 9:
                /* toggle focus between win_form and win_out*/

                if (focus == FOCUS_RIGHT && n_out_choices!=0){
                     focus = FOCUS_LEFT;
                    (void) snprintf(buffer, 128, "focus in window %d", focus);
                }
                else if(focus == FOCUS_LEFT && n_out_choices!=0){
                    focus = FOCUS_RIGHT;
                    (void) snprintf(buffer, 128, "focus in window %d", focus);
                }
                else{
                    (void) snprintf(buffer, 128, "No hay resultados que inspeccionar");
                }

                write_msg(msg_win, buffer, -1, -1, windows->msg_title);
                /* If win_form is selected place the cursor in the right place */
                if (item_index(auxItem) == SEARCH && focus == FOCUS_LEFT) {
                    (void)form_driver(forms->search_form, REQ_END_LINE);
                    (void)wrefresh(windows->form_search_win);
                }
                else if (item_index(auxItem) == BPASS && focus == FOCUS_LEFT) {
                    (void)form_driver(forms->bpass_form, REQ_END_LINE);
                    (void)wrefresh(windows->form_bpass_win);
                }
                break;
            case KEY_BACKSPACE: /* delete last key */
            case 127:
                if (item_index(auxItem) == SEARCH) {
                    (void)form_driver(forms->search_form, REQ_DEL_PREV);
                    (void)wrefresh(windows->form_search_win);
                } else if (item_index(auxItem) == BPASS) {
                    (void)form_driver(forms->bpass_form, REQ_DEL_PREV);
                    (void)wrefresh(windows->form_bpass_win);
                }
                break;
            case 10: /* enter has been pressed*/
                auxItem = current_item(menu);
                choice = item_index(auxItem);
                enterKey = (bool) TRUE; /* mark enter pressed */
                break;
            case 338: /* Av Página has been pressed*/
            if (focus == FOCUS_RIGHT && n_out_choices!=0){
                    des+=max_rows;
                    page=MIN(page+1,max_pages);
                    if(des>=n_out_choices){
                        des-=max_rows;
                    }
                (void) wclear(out_win);
                    out_highlight = 0+des;
                    print_out(out_win, menus->out_win_choices,1+des,MIN(max_rows, n_out_choices) +des,
                                out_highlight, windows->out_title);
                }
                break;
            case 339: /* Re Página has been pressed*/
                if(focus == FOCUS_RIGHT && n_out_choices!=0){
                    des-=max_rows;
                    page=MAX(page-1,1);
                    if(des<=0){
                        des=0;
                    }
                (void) wclear(out_win);
                    out_highlight = 0+des;
                    print_out(out_win, menus->out_win_choices,1+des, MIN(max_rows, n_out_choices)+des,
                                out_highlight, windows->out_title);
                }
                break;
            
            default: /* echo pressed key */
                auxItem = current_item(menu);
                if (item_index(auxItem) == SEARCH) {
                    (void)form_driver(forms->search_form, ch);
                    (void)wrefresh(windows->form_search_win);
                }
                else if (item_index(auxItem) == BPASS) {
                    (void)form_driver(forms->bpass_form, ch);
                    (void)wrefresh(windows->form_bpass_win);
                }
                break;
        }

        if (choice != -1 && enterKey) /* User did a choice process it */
        {
            if (choice == QUIT)
                break; /* quit */
            else if ((choice == SEARCH) && (focus == FOCUS_LEFT)) {
                out_highlight = 0;
                for(i=0; i< rows_out_window ; i++)
                    (menus->out_win_choices)[i][0] = '\0';
                (void)wclear(out_win);
                (void)form_driver(forms->search_form, REQ_VALIDATION);
                tmpStr1 = field_buffer((forms->search_form_items)[1], 0);
                tmpStr2 = field_buffer((forms->search_form_items)[3], 0);
                tmpStr3 = field_buffer((forms->search_form_items)[5], 0);
                results_search(tmpStr1, tmpStr2, tmpStr3, &n_out_choices, & (menus->out_win_choices), & (menus->out_win_choices_info),
                               windows->cols_out_win-4, windows->rows_out_win-2);
                if (n_out_choices==0)
                {
                    page=0;
                }
                else{
                page=1;
                }
                des=0;
                if(n_out_choices%max_rows==0){
                        max_pages=n_out_choices/max_rows;
                    }
                    else{
                        max_pages=(n_out_choices/max_rows)+1;
                    }
                rows_last_page=n_out_choices%max_rows;
                        if(rows_last_page==0    ){
                            rows_last_page=max_rows;
                        }
                print_out(out_win, menus->out_win_choices,1, MIN(max_rows, n_out_choices),
                          out_highlight, windows->out_title);

                if ((bool)DEBUG) {

                    if(n_out_choices == 0){
                        snprintf(buffer, 128, "No se encontraron vuelos con los datos introducidos. Reviselos para realizar su búsqueda");
                    }
                    else{
                        (void)snprintf(buffer, 128, "Busqueda encontrada .arg1=%s, arg2=%s, arg3= %s",  tmpStr1, tmpStr2, tmpStr3);
                    }
                write_msg(msg_win, buffer, -1, -1, windows->msg_title);
                }

            }
            else if ((choice == SEARCH) && (focus == FOCUS_RIGHT)) {
                (void)snprintf(buffer, 128, "msg=%s", (menus->out_win_choices_info)[out_highlight] );
                write_msg(msg_win,buffer,
                          -1, -1, windows->msg_title);
            }
            else if ((choice == BPASS) && (focus == FOCUS_LEFT)) {
                out_highlight = 0;
                for(i=0; i< rows_out_window ; i++)
                    (menus->out_win_choices)[i][0] = '\0';
                (void) wclear(out_win);
                (void) form_driver(forms->bpass_form, REQ_VALIDATION);
                tmpStr1 = field_buffer((forms->bpass_form_items)[1], 0);
                results_bpass(tmpStr1, &n_out_choices, & (menus->out_win_choices),
                              windows->cols_out_win-4, windows->rows_out_win-2);
                if (n_out_choices==0)
                {
                    page=0;
                }
                else{
                page=1;
                }
                des=0;
                if(n_out_choices%max_rows==0){
                        max_pages=n_out_choices/max_rows;
                    }
                    else{
                        max_pages=(n_out_choices/max_rows)+1;
                    }
                rows_last_page=n_out_choices%max_rows;
                if(rows_last_page==0){
                            rows_last_page=max_rows;
                }
                print_out(out_win, menus->out_win_choices,1, MIN(max_rows, n_out_choices),
                          out_highlight, windows->out_title);
            }
            else if ((choice == BPASS) && focus == (FOCUS_RIGHT)) {
                write_msg(msg_win, (menus->out_win_choices)[out_highlight],
                          -1, -1, windows->msg_title);
            }
        }
        choice = -1;
        enterKey = (bool) FALSE;
    }
}
