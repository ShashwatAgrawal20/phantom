#ifndef PHANTOM_H
#define PHANTOM_H

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int convert_file(const char *md_path, const char *html_path);
static void print_help(const char *program_name);
static void _inline_formatting(FILE *html_file, const char *text);

typedef enum {
    OUTSIDE,
    IN_PARAGRAPH,
    IN_CODE_BLOCK,
    IN_ORDERED_LIST,
    IN_UNORDERED_LIST
} parser_state_e;

#define return_defer(value) \
    do {                    \
        result = (value);   \
        goto defer;         \
    } while (0)

static void print_help(const char *program_name) {
    printf("Phantom Markdown to HTML Converter\n");
    printf("\nUsage:\n");
    printf(
        "  %s <input.md> <output.html>   Convert a single Markdown file to "
        "HTML\n",
        program_name);
    printf(
        "  %s <input_directory>          Convert all Markdown files in a "
        "directory (WIP)\n",
        program_name);
    printf("\nOptions:\n");
    printf("  --help   Show this help message and exit\n");
}

static void _inline_formatting(FILE *html_file, const char *text) {
    bool in_bold = false, in_italic = false, in_code = false;

    for (const char *c = text; *c; c++) {
        if (*c == '`' && !in_bold && !in_italic) {
            if (!in_code) {
                fprintf(html_file, "<code>");
                in_code = true;
            } else {
                fprintf(html_file, "</code>");
                in_code = false;
            }
        } else if (*c == '*' && c[1] == '*' && !in_code) {
            if (!in_bold) {
                fprintf(html_file, "<strong>");
                in_bold = true;
            } else {
                fprintf(html_file, "</strong>");
                in_bold = false;
            }
            c++;
        } else if (*c == '*' && c[1] != '*' && !in_code && !in_bold) {
            if (!in_italic) {
                fprintf(html_file, "<em>");
                in_italic = true;
            } else {
                fprintf(html_file, "</em>");
                in_italic = false;
            }
        } else {
            if (*c == '<')
                fprintf(html_file, "&lt;");
            else if (*c == '>')
                fprintf(html_file, "&gt;");
            else if (*c == '&')
                fprintf(html_file, "&amp;");
            else if (*c == '"')
                fprintf(html_file, "&quot;");
            else
                fprintf(html_file, "%c", *c);
        }
    }

    if (in_bold) fprintf(html_file, "</strong>");
    if (in_italic) fprintf(html_file, "</em>");
    if (in_code) fprintf(html_file, "</code>");
}

static inline int convert_file(const char *md_path, const char *html_path) {
    FILE *md_file = NULL, *html_file = NULL;
    char *line = NULL;
    size_t len = 0;
    int result = 0;
    parser_state_e state = OUTSIDE;
    int list_item_number = 1;

    md_file = fopen(md_path, "r");
    if (!md_file) {
        fprintf(stderr, "Error opening %s: %s\n", md_path, strerror(errno));
        return_defer(1);
    }

    html_file = fopen(html_path, "w");
    if (!html_file) {
        fprintf(stderr, "Error: Cannot create %s\n", html_path);
        return_defer(1);
    }

    fprintf(html_file,
            "<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "    <meta charset=\"UTF-8\">\n"
            "    <meta name=\"viewport\" content=\"width=device-width, "
            "initial-scale=1.0\">\n"
            "    <title>%s</title>\n"
            "    <style>\n"
            "        body {\n"
            "            font-family: system-ui, -apple-system, sans-serif;\n"
            "            max-width: 800px;\n"
            "            margin: 0 auto;\n"
            "            padding: 20px;\n"
            "            line-height: 1.6;\n"
            "        }\n"
            "        pre {\n"
            "            background-color: #f5f5f5;\n"
            "            padding: 1rem;\n"
            "            border-radius: 4px;\n"
            "            overflow: auto;\n"
            "        }\n"
            "        code {\n"
            "            font-family: 'SF Mono', Consolas, monospace;\n"
            "            background-color: #f5f5f5;\n"
            "            padding: 0.2rem 0.4rem;\n"
            "            border-radius: 3px;\n"
            "        }\n"
            "    </style>\n"
            "</head>\n"
            "<body>\n",
            md_path);

    while (getline(&line, &len, md_file) != -1) {
        line[strcspn(line, "\r\n")] = 0;

        int is_new_block =
            (*line == '\0' || *line == '#' || (strncmp(line, "```", 3) == 0) ||
             (strncmp(line, "- ", 2) == 0) ||
             (isdigit(*line) &&
              strstr(line, ". ") == line + strspn(line, "0123456789")));

        if (state == IN_PARAGRAPH && is_new_block) {
            fprintf(html_file, "</p>\n");
            state = OUTSIDE;
        }

        if ((state == IN_ORDERED_LIST || state == IN_UNORDERED_LIST) &&
            !(line[0] == '-' && line[1] == ' ') && !isdigit(line[0]) &&
            !(line[1] == '.' && line[2] == ' ')) {
            if (state == IN_ORDERED_LIST) fprintf(html_file, "</ol>\n");
            if (state == IN_UNORDERED_LIST) fprintf(html_file, "</ul>\n");
            list_item_number = 1;
            state = OUTSIDE;
        }

        if (strncmp(line, "```", 3) == 0) {
            if (state != IN_CODE_BLOCK) {
                state = IN_CODE_BLOCK;
                fprintf(html_file, "<pre><code>");
            } else {
                state = OUTSIDE;
                fprintf(html_file, "</code></pre>\n");
            }
            continue;
        } else if (state == IN_CODE_BLOCK) {
            for (const char *c = line; *c; c++) {
                if (*c == '<')
                    fprintf(html_file, "&lt;");
                else if (*c == '>')
                    fprintf(html_file, "&gt;");
                else if (*c == '&')
                    fprintf(html_file, "&amp;");
                else if (*c == '"')
                    fprintf(html_file, "&quot;");
                else
                    fprintf(html_file, "%c", *c);
            }
            fprintf(html_file, "\n");
            continue;
        } else if (line[0] == '\0') {
            continue;
        } else if (line[0] == '#') {
            int level = 0;
            while (level < 6 && line[level] == '#') level++;
            if (level > 0 && line[level] == ' ') {
                fprintf(html_file, "<h%d>", level);
                _inline_formatting(html_file, line + level + 1);
                fprintf(html_file, "</h%d>\n", level);
            }
        } else if (line[0] == '-' && line[1] == ' ') {
            if (state != IN_UNORDERED_LIST) {
                state = IN_UNORDERED_LIST;
                fprintf(html_file, "<ul>\n");
            }
            fprintf(html_file, "<li>");
            _inline_formatting(html_file, line + 2);
            fprintf(html_file, "</li>\n");
        } else if (isdigit(line[0])) {
            char *num_end;
            int number = strtol(line, &num_end, 10);
            if (*num_end == '.' && num_end[1] == ' ') {
                if (state != IN_ORDERED_LIST) {
                    state = IN_ORDERED_LIST;
                    list_item_number = number;
                    fprintf(html_file, "<ol start=\"%d\">\n", list_item_number);
                }
                fprintf(html_file, "<li>");
                _inline_formatting(html_file, num_end + 2);
                fprintf(html_file, "</li>\n");
                list_item_number++;
            }
        } else {
            if (state != IN_PARAGRAPH) {
                state = IN_PARAGRAPH;
                fprintf(html_file, "<p>\n");
                _inline_formatting(html_file, line);
                fprintf(html_file, "\n");
                continue;
            }
            _inline_formatting(html_file, line);
        }
    }

    if (state == IN_PARAGRAPH) fprintf(html_file, "</p>\n");
    if (state == IN_ORDERED_LIST) fprintf(html_file, "</ol>\n");
    if (state == IN_UNORDERED_LIST) fprintf(html_file, "</ul>\n");
    if (state == IN_CODE_BLOCK) fprintf(html_file, "</code></pre>\n");

    fprintf(html_file, "</body>\n</html>\n");

defer:
    if (md_file) fclose(md_file);
    if (html_file) fclose(html_file);
    free(line);
    return result;
}
#endif
