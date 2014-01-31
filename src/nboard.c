/**
 * @file nboard.c
 *
 * Chris Welty's NBoard Protocol
 *
 * @date 1998 - 2013
 * @author Richard Delorme
 * @version 4.4
 */

#include "options.h"
#include "search.h"
#include "util.h"
#include "ui.h"
#include "play.h"

#include <stdarg.h>

static Log nboard_log[1];

/**
 * Send data to the gui.
 * @param format string format.
 * @param ... data to output.
 */
static void nboard_send(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	putchar('\n'); fflush(stdout);

	if (log_is_open(nboard_log)) {
		fprintf(nboard_log->f, "sent> \"");
		va_start(args, format);
		vfprintf(nboard_log->f, format, args);
		va_end(args);
		fprintf(nboard_log->f, "\"\n");
	}
}

/**
 * Send an error to the gui.
 * @param format string format.
 * @param ... data to output.
 */
static void nboard_fail(const char *format, ...)
{
	va_list args;

	fprintf(stderr, "Error: ");
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	putc('\n', stderr); fflush(stderr);

	if (log_is_open(nboard_log)) {
		fprintf(nboard_log->f, "error> \"");
		va_start(args, format);
		vfprintf(nboard_log->f, format, args);
		va_end(args);
		fprintf(nboard_log->f, "\"\n");
	}
}

/**
 * Send a move to the gui.
 * @param Result search result
 */
static void nboard_send_move(Result *result)
{
	char move[4];

	move_to_string(result->move, WHITE, move);
	nboard_send("=== %s %.2f %.1f", move, 1.0 * result->score, 0.001 * result->time);
}

/**
 * @brief Parse a ggf game from a string.
 *
 * @param ui User interface
 * @param string Input string.
 * @return The unprocessed remaining part of the string.
 */
static char* nboard_set_game(UI *ui, const char *string)
{
	const char *s = string;
	const char *next;
	char tag[4], value[256];
	Play *play = ui->play;

	while ((next = parse_tag(s, tag, value)) != s && strcmp(tag, "(;") != 0) s = next;

	if (strcmp(tag, "(;") == 0) {
		s = next;
		while ((next = parse_tag(s, tag, value)) != s && strcmp(tag, ";)") != 0) {
			s = next;

			if (strcmp(tag, "GM") == 0 && strcmp(value, "othello") != 0) {
				s = string;
				break;
			} else if (strcmp(tag, "BO") == 0) {
				if (value[0] != '8') {
					s = string;
					break;
				}
				play_set_board(play, value + 2);
			} else if (strcmp(tag, "PB") == 0) {
			} else if (strcmp(tag, "PW") == 0) {
			} else if ((strcmp(tag, "B") == 0 || strcmp(tag, "W") == 0)) {
				if (!play_move(play, string_to_coordinate(value))) {
					nboard_fail("illegal move %s[%s] in set game", tag, value);
				}
			}
		}
	}

	return (char*) s;
}


/**
 * Nboard search observer
 * @param Result search result
 */
static void nboard_observer(Result *result)
{
	if (log_is_open(nboard_log)) {
		fprintf(nboard_log->f, "edax> ");
		result_print(result, nboard_log->f);
		putc('\n', nboard_log->f);
	}
	nboard_send("nodestats %lld %.2f", result->n_nodes, result->time);
}

/**
 * @brief initialize edax protocol
 * @param ui user interface
 */
void ui_init_nboard(UI *ui)
{
	Play *play = ui->play;

	// other update...
	play_init(play, ui->book);
	play->search->options.header = play->search->options.separator = NULL;
	ui->book->search = play->search;
	book_load(ui->book, options.book_file);
	play->search->id = 1;
	search_set_observer(play->search, nboard_observer);
	ui->mode = 3;
	play->type = ui->type;

	log_open(nboard_log, options.ui_log_file);
}

/**
 * @brief free resources used by edax protocol
 * @param ui user interface
 */
void ui_free_nboard(UI *ui)
{
	if (ui->book->need_saving) book_save(ui->book, options.book_file);
	book_free(ui->book);
	play_free(ui->play);
	log_close(nboard_log);
}

/**
 * @brief Loop event
 * @param ui user interface
 */
void ui_loop_nboard(UI *ui)
{
	char *cmd = NULL, *param = NULL;
	Play *play = ui->play;

	// loop forever
	for (;;) {
		errno = 0;

		if (log_is_open(nboard_log)) {
			play_print(play, nboard_log->f);
		}

		ui_event_wait(ui, &cmd, &param);
		log_print(nboard_log, "received< \"%s %s\"\n", cmd, param);

		if (*cmd == '\0') {

		} else if (strcmp(cmd, "nboard") == 0) {
			if (strcmp(param, "1") != 0) nboard_fail("Edax expected \"nboard 1\" protocol");

		} else if (strcmp(cmd, "depth") == 0) {
			options.level = string_to_int(param, 21);
			nboard_send("set myname Edax%d", options.level);

		} else if (strcmp(cmd, "game") == 0) {
			if (nboard_set_game(ui, param) == param) {
				nboard_fail("Cannot parse game \"%s\"", param);
			}

		} else if (strcmp(cmd, "move") == 0) {
			if (!play_user_move(play, param)) {
				nboard_fail("Cannot parse move \"%s\"", param);
			}
		} else if (strcmp(cmd, "hint") == 0) {

			nboard_send("status Edax is thinking");
			play_hint(play, string_to_int(param, MAX_MOVE));
			nboard_send("status Edax is waiting");

		} else if (strcmp(cmd, "go") == 0) {
			nboard_send("status Edax is thinking");
			play_go(play, false);
			nboard_send_move(play->result);
			nboard_send("status Edax is waiting");

		} else if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "eof") == 0) {
			free(cmd); free(param);			
			return;

		} else if (strcmp(cmd, "ping") == 0) {
			nboard_send("pong %s", param);

		} else if (strcmp(cmd, "learn") == 0) {
			nboard_send("status Edax is learning");
			play_store(play);
			nboard_send("status Edax is waiting");

		} else if (play_user_move(play, cmd)) {

		// error: unknown message
		} else {
			nboard_fail("unknown command \"%s\" \"%s\"", cmd, param);
		}
	}
}


