#include <stdio.h>
#include <string.h>

int convert_file(const char *md_path, const char *html_path) {
    FILE *md_file, *html_file;
    char line[1024];
    int in_paragraph = 0;

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

    fprintf(
        html_file,
        "<!DOCTYPE html>\n<html>\n<head>\n  <title>Generated from %s</title>\n",
        md_path);
    fprintf(html_file,
            "  <style>body{font-family:sans-serif;max-width:800px;margin:0 "
            "auto;padding:20px;}</style>\n");
    fprintf(html_file, "</head>\n<body>\n");

    while (fgets(line, sizeof(line), md_file)) {
        line[strcspn(line, "\r\n")] = 0;

        int is_new_block =
            (*line == '\0' || *line == '#' || (*line == '-' && line[1] == ' '));

        if (in_paragraph && is_new_block) {
            fprintf(html_file, "</p>\n");
            in_paragraph = 0;
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
            fprintf(html_file, "<li>%s</li>\n", line + 2);
        } else {
            if (!in_paragraph) {
                in_paragraph = 1;
                fprintf(html_file, "<p>\n%s\n", line);
                continue;
            }
            fprintf(html_file, "%s\n", line);
        }
    }
    if (in_paragraph) {
        fprintf(html_file, "</p>\n");
    }

    fprintf(html_file, "</body>\n</html>\n");

    fclose(md_file);
    fclose(html_file);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 2) {
        printf("Usage:\n");
        printf("  %s <input.md> <output.html>  - convert single file\n",
               argv[0]);
        printf(
            "  %s <input_directory>         - convert all .md files in "
            "directory\n",
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
