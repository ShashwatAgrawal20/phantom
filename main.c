#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    OUTSIDE,
    IN_PARAGRAPH,
    IN_CODE_BLOCK,
    IN_ORDERED_LIST,
    IN_UNORDERED_LIST
} parser_state_e;

int convert_file(const char *md_path, const char *html_path) {
    FILE *md_file, *html_file;
    char line[1024];
    parser_state_e state = OUTSIDE;
    int list_item_number = 1;

    md_file = fopen(md_path, "r");
    if (!md_file) {
        fprintf(stderr, "Error: cannot open %s\n", md_path);
        return 1;
    }

    html_file = fopen(html_path, "w");
    if (!html_file) {
        fprintf(stderr, "Error: Cannot create %s\n", html_path);
        fclose(md_file);
        return 1;
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

    while (fgets(line, sizeof(line), md_file)) {
        line[strcspn(line, "\r\n")] = 0;

        if (strncmp(line, "```", 3) == 0) {
            if (state != IN_CODE_BLOCK) {
                if (state == IN_PARAGRAPH) {
                    fprintf(html_file, "</p>\n");
                }
                state = IN_CODE_BLOCK;
                fprintf(html_file, "<pre><code>");
            } else {
                state = OUTSIDE;
                fprintf(html_file, "</code></pre>\n");
            }
            continue;
        }

        if (state == IN_CODE_BLOCK) {
            for (char *c = line; *c; c++) {
                if (*c == '<')
                    fprintf(html_file, "&lt;");
                else if (*c == '>')
                    fprintf(html_file, "&gt;");
                else if (*c == '&')
                    fprintf(html_file, "&amp;");
                else
                    fprintf(html_file, "%c", *c);
            }
            fprintf(html_file, "\n");
            continue;
        }

        int is_new_block =
            (*line == '\0' || *line == '#' || (*line == '-' && line[1] == ' '));

        if (state == IN_PARAGRAPH && is_new_block) {
            fprintf(html_file, "</p>\n");
            state = OUTSIDE;
        }

        if ((state == IN_ORDERED_LIST || state == IN_UNORDERED_LIST) &&
            !(line[0] == '-' && line[1] == ' ') &&
            !(isdigit(line[0]) && line[1] == '.' && line[2] == ' ') &&
            line[0] != '\0') {
            if (state == IN_ORDERED_LIST) {
                fprintf(html_file, "</ol>\n");
                state = OUTSIDE;
                list_item_number = 1;
            }
            if (state == IN_UNORDERED_LIST) {
                fprintf(html_file, "</ul>\n");
                state = OUTSIDE;
            }
        }

        if (line[0] == '\0') {
            continue;
        } else if (line[0] == '#') {
            int level = 0;
            while (level < 6 && line[level] == '#') level++;
            if (level > 0 && line[level] == ' ') {
                fprintf(html_file, "<h%d>%s</h%d>\n", level, line + level + 1,
                        level);
            } else {
                fprintf(html_file, "<p>%s</p>\n", line);
            }
        } else if (line[0] == '-' && line[1] == ' ') {
            if (state != IN_UNORDERED_LIST) {
                state = IN_UNORDERED_LIST;
                fprintf(html_file, "<ul>\n");
            }
            fprintf(html_file, "<li>%s</li>\n", line + 2);
        } else if (isdigit(line[0]) && line[1] == '.' && line[2] == ' ') {
            if (state != IN_ORDERED_LIST) {
                state = IN_ORDERED_LIST;
                list_item_number = atoi(line);
                fprintf(html_file, "<ol start=\"%d\">\n", list_item_number);
            }
            fprintf(html_file, "<li>%s</li>\n", line + 3);
            list_item_number++;
        } else {
            if (state != IN_PARAGRAPH) {
                state = IN_PARAGRAPH;
                fprintf(html_file, "<p>\n%s\n", line);
                continue;
            }
            fprintf(html_file, "%s\n", line);
        }
    }

    if (state == IN_PARAGRAPH) fprintf(html_file, "</p>\n");
    if (state == IN_ORDERED_LIST) fprintf(html_file, "</ol>\n");
    if (state == IN_UNORDERED_LIST) fprintf(html_file, "</ul>\n");
    if (state == IN_CODE_BLOCK) fprintf(html_file, "</code></pre>\n");

    fprintf(html_file, "</body>\n</html>\n");
    fclose(md_file);
    fclose(html_file);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 2) {
        printf("Usage:\n");
        printf(" %s <input.md> <output.html> - convert single file\n", argv[0]);
        printf(" %s <input_directory> - convert all .md files in directory\n",
               argv[0]);
        return 1;
    }

    if (argc == 3) {
        if (convert_file(argv[1], argv[2])) {
            fprintf(stderr, "Failed converting %s to %s\n", argv[1], argv[2]);
            return 1;
        }
        printf("Converted %s to %s\n", argv[1], argv[2]);
    }

    return 0;
}
