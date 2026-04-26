#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include "libtermcolor.h"
#include "tc_file_types.h"
#include "tc_file_directories_color.h"

static const char *system_extensions[] = {
    "so", "dll", "ko", "sys", "exe", "bin", "app", "run", "dylib", NULL
};

static const char *text_extensions[] = {
    "txt", "text", "readme", "license", "copying", "md", "rst", "log", NULL
};

static const char *c_extensions[] = {
    "c", "h", NULL
};

static const char *cpp_extensions[] = {
    "cpp", "cc", "cxx", "hpp", "hh", "hxx", "c++", "h++", NULL
};

static const char *python_extensions[] = {
    "py", "pyc", "pyo", "pyd", "pyw", "pyi", NULL
};

static const char *js_extensions[] = {
    "js", "jsx", "ts", "tsx", "mjs", "cjs", NULL
};

static const char *java_extensions[] = {
    "java", "class", "jar", "war", NULL
};

static const char *go_extensions[] = {
    "go", NULL
};
static const char *rust_extensions[] = {
    "rs", "rlib", NULL
};

static const char *html_extensions[] = {
    "html", "htm", "xhtml", "shtml", NULL
};

static const char *css_extensions[] = {
    "css", "scss", "sass", "less", NULL
};

static const char *json_extensions[] = {
    "json", "jsonl", "json5", NULL
};

static const char *xml_extensions[] = {
    "xml", "xsl", "xslt", "xsd", NULL
};

static const char *yaml_extensions[] = {
    "yaml", "yml", NULL
};

static const char *config_extensions[] = {
    "cfg", "conf", "ini", "toml", "properties", "env", NULL
};

static const char *script_extensions[] = {
    "sh", "bash", "zsh", "fish", "ps1", "bat", "cmd", NULL
};

static const char *document_extensions[] = {
    "pdf", "doc", "docx", "odt", "rtf", "tex", "epub", "mobi", NULL
};

static const char *image_extensions[] = {
    "jpg", "jpeg", "png", "gif", "bmp", "tiff", "tif",
    "webp", "ico", "icns", "psd", "xcf", NULL
};

static const char *image_raw_extensions[] = {
    "raw", "nef", "cr2", "arw", "dng", "orf", "rw2", NULL
};

static const char *image_vector_extensions[] = {
    "svg", "ai", "eps", "ps", NULL
};

static const char *audio_pcm_extensions[] = {
    "pcm", NULL
};

static const char *audio_extensions[] = {
    "mp3", "wav", "flac", "ogg", "opus", "aac", "m4a", "wma", "alac", "ape", "dsd", "mid", "midi", NULL
};

static const char *video_extensions[] = {
    "mp4", "avi", "mkv", "mov", "wmv", "flv", "webm",
    "mpeg", "mpg", "m4v", "3gp", "ogv", "vob", NULL
};

static const char *archive_extensions[] = {
    "zip", "tar", "gz", "bz2", "xz", "zst", "lz", "lzma", NULL
};

static const char *archive_rar_extensions[] = {
    "rar", NULL
};

static const char *archive_7z_extensions[] = {
    "7z", NULL
};

static const char *database_extensions[] = {
    "db", "sqlite", "sqlite3", "mdb", "accdb", "dbf", NULL
};

static const char *spreadsheet_extensions[] = {
    "xls", "xlsx", "ods", "csv", "tsv", NULL
};

static const char *presentation_extensions[] = {
    "ppt", "pptx", "odp", "key", NULL
};

static const char *font_extensions[] = {
    "ttf", "otf", "woff", "woff2", "eot", NULL
};

static const char *binary_extensions[] = {
    "bin", "dat", "dump", "img", "iso", NULL
};

static const char *temp_extensions[] = {
    "tmp", "temp", "bak", "backup", "swp", "swo", NULL
};

static const char *virtual_extensions[] = {
    "proc", "sys", "dev", NULL
};

static const char *markdown_extensions[] = {
    "md", "markdown", "mdown", NULL
};

static const char *raw_extensions[] = {
    "raw", NULL
};

static const char *const *extension_groups[FILE_TYPE_COUNT] = {
    [FILE_TYPE_SYSTEM] = system_extensions,
    [FILE_TYPE_TEXT] = text_extensions,
    [FILE_TYPE_C] = c_extensions,
    [FILE_TYPE_CPP] = cpp_extensions,
    [FILE_TYPE_PYTHON] = python_extensions,
    [FILE_TYPE_JAVASCRIPT] = js_extensions,
    [FILE_TYPE_JAVA] = java_extensions,
    [FILE_TYPE_GO] = go_extensions,
    [FILE_TYPE_RUST] = rust_extensions,
    [FILE_TYPE_HTML] = html_extensions,
    [FILE_TYPE_CSS] = css_extensions,
    [FILE_TYPE_JSON] = json_extensions,
    [FILE_TYPE_XML] = xml_extensions,
    [FILE_TYPE_YAML] = yaml_extensions,
    [FILE_TYPE_CONFIG] = config_extensions,
    [FILE_TYPE_SCRIPT] = script_extensions,
    [FILE_TYPE_DOCUMENT] = document_extensions,
    [FILE_TYPE_IMAGE] = image_extensions,
    [FILE_TYPE_IMAGE_RAW] = image_raw_extensions,
    [FILE_TYPE_IMAGE_VECTOR] = image_vector_extensions,
    [FILE_TYPE_AUDIO] = audio_extensions,
    [FILE_TYPE_AUDIO_PCM] = audio_pcm_extensions,
    [FILE_TYPE_VIDEO] = video_extensions,
    [FILE_TYPE_ARCHIVE] = archive_extensions,
    [FILE_TYPE_ARCHIVE_RAR] = archive_rar_extensions,
    [FILE_TYPE_ARCHIVE_7Z] = archive_7z_extensions,
    [FILE_TYPE_DATABASE] = database_extensions,
    [FILE_TYPE_SPREADSHEET] = spreadsheet_extensions,
    [FILE_TYPE_PRESENTATION] = presentation_extensions,
    [FILE_TYPE_FONT] = font_extensions,
    [FILE_TYPE_BINARY] = binary_extensions,
    [FILE_TYPE_TEMP] = temp_extensions,
    [FILE_TYPE_VIRTUAL] = virtual_extensions,
    [FILE_TYPE_MARKDOWN] = markdown_extensions,
    [FILE_TYPE_AMBIGUOUS_RAW] = raw_extensions
};

 int extension_in_list(const char *ext, const char *const *list) {
    if (!ext || !list) return 0;
    size_t i;
    for (i = 0; i < 256; i++) {
        if (ext[i] == '\0') break;
        if (ext[i] < 32 || ext[i] > 126) {
            return 0;
        }
    }
    if (i == 0 || i >= 256) return 0;
    for (int j = 0; list[j] != NULL; j++) {
        if (strcasecmp(ext, list[j]) == 0) {
            return 1;
        }
    }
    return 0;
}

FileType get_file_type(const char *filename) {
    if (!filename) return FILE_TYPE_UNKNOWN;
    uintptr_t ptr = (uintptr_t)filename;
    if (ptr < 0x1000) return FILE_TYPE_UNKNOWN;
    size_t len = 0;
    for (len = 0; len < 256; len++) {
        if (filename[len] == '\0') break;
    }
    if (len == 0 || len >= 256) return FILE_TYPE_UNKNOWN;
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return FILE_TYPE_UNKNOWN;
    const char *ext = dot + 1;
    size_t ext_len = strlen(ext);
    if (ext_len == 0 || ext_len > 20) return FILE_TYPE_UNKNOWN;
    for (FileType type = FILE_TYPE_SYSTEM; type < FILE_TYPE_COUNT; type++) {
        const char *const *group = extension_groups[type];
        if (group && extension_in_list(ext, group)) {
            return type;
        }
    }
    return FILE_TYPE_UNKNOWN;
}

const char *get_file_type_name(FileType type) {
    static const char *names[FILE_TYPE_COUNT] = {
        [FILE_TYPE_UNKNOWN] = "Unknown",
        [FILE_TYPE_SYSTEM] = "System",
        [FILE_TYPE_TEXT] = "Text",
        [FILE_TYPE_C] = "C",
        [FILE_TYPE_CPP] = "C++",
        [FILE_TYPE_PYTHON] = "Python",
        [FILE_TYPE_JAVASCRIPT] = "JavaScript",
        [FILE_TYPE_JAVA] = "Java",
        [FILE_TYPE_GO] = "Go",
        [FILE_TYPE_RUST] = "Rust",
        [FILE_TYPE_HTML] = "HTML",
        [FILE_TYPE_CSS] = "CSS",
        [FILE_TYPE_JSON] = "JSON",
        [FILE_TYPE_XML] = "XML",
        [FILE_TYPE_YAML] = "YAML",
        [FILE_TYPE_CONFIG] = "Config",
        [FILE_TYPE_SCRIPT] = "Script",
        [FILE_TYPE_DOCUMENT] = "Document",
        [FILE_TYPE_IMAGE] = "Image",
        [FILE_TYPE_IMAGE_RAW] = "Raw Image",
        [FILE_TYPE_IMAGE_VECTOR] = "Vector Image",
        [FILE_TYPE_AUDIO] = "Audio",
        [FILE_TYPE_AUDIO_PCM] = "Pcm Audio",
        [FILE_TYPE_VIDEO] = "Video",
        [FILE_TYPE_ARCHIVE] = "Archive",
        [FILE_TYPE_ARCHIVE_RAR] = "RAR Archive",
        [FILE_TYPE_ARCHIVE_7Z] = "7z Archive",
        [FILE_TYPE_DATABASE] = "Database",
        [FILE_TYPE_SPREADSHEET] = "Spreadsheet",
        [FILE_TYPE_PRESENTATION] = "Presentation",
        [FILE_TYPE_FONT] = "Font",
        [FILE_TYPE_BINARY] = "Binary",
        [FILE_TYPE_TEMP] = "Temporary",
        [FILE_TYPE_VIRTUAL] = "Virtual",
        [FILE_TYPE_MARKDOWN] = "Markdown",
        [FILE_TYPE_DIRECTORY] = "Directory",
        [FILE_TYPE_AMBIGUOUS_RAW] = "Raw extensions"
    };
    if (type >= 0 && type < FILE_TYPE_COUNT) {
        return names[type];
    }
    return "Invalid";
}

int is_file_type(const char *filename, FileType type) {
    return (get_file_type(filename) == type);
}

int is_file_category(const char *filename, unsigned int category_flags) {
    if (!filename || !category_flags) {
        return 0;
    }
    FileType type = get_file_type(filename);
    if (category_flags & FILE_CATEGORY_TEXT) {
        switch (type) {
            case FILE_TYPE_TEXT:
            case FILE_TYPE_C:
            case FILE_TYPE_CPP:
            case FILE_TYPE_PYTHON:
            case FILE_TYPE_JAVASCRIPT:
            case FILE_TYPE_JAVA:
            case FILE_TYPE_GO:
            case FILE_TYPE_RUST:
            case FILE_TYPE_HTML:
            case FILE_TYPE_CSS:
            case FILE_TYPE_JSON:
            case FILE_TYPE_XML:
            case FILE_TYPE_YAML:
            case FILE_TYPE_CONFIG:
            case FILE_TYPE_SCRIPT:
            case FILE_TYPE_MARKDOWN:
                return 1;
            default:
                break;
        }
    }
    if (category_flags & FILE_CATEGORY_PCM) {
        if (type == FILE_TYPE_AUDIO_PCM) {
            return 1;
        }
    }

    if (category_flags & FILE_CATEGORY_IMAGE) {
        switch (type) {
            case FILE_TYPE_IMAGE:
            case FILE_TYPE_IMAGE_VECTOR:
                return 1;
            default:
                break;
        }
    }
    if (category_flags & FILE_CATEGORY_SYSTEM) {
        if (type == FILE_TYPE_SYSTEM) {
            return 1;
        }
    }
    if (category_flags & FILE_CATEGORY_RAW) {
        if (type == FILE_TYPE_AMBIGUOUS_RAW) {
            return 1;
        }
    }
    if (category_flags & FILE_CATEGORY_ARCHIVE) {
        switch (type) {
            case FILE_TYPE_ARCHIVE:
            case FILE_TYPE_ARCHIVE_RAR:
            case FILE_TYPE_ARCHIVE_7Z:
                return 1;
            default:
                break;
        }
    }
    if (category_flags & FILE_CATEGORY_AUDIO) {
        switch (type) {
            case FILE_TYPE_AUDIO:
                return 1;
            default:
                break;
        }
    }

    if (category_flags & FILE_CATEGORY_VIDEO) {
        if (type == FILE_TYPE_VIDEO) {
            return 1;
        }
    }
    return 0;
}

int get_text_module_file_color(const char *filename, int is_dir, char flag) {
    if (is_dir) {
        return get_color_pair(6, -1);
    }
    if (is_file_category(filename, FILE_CATEGORY_TEXT)) {
        return (flag == 'a') ? get_color_pair(5, -1) : get_color_pair(249, -1);
    }
    if (is_file_category(filename, FILE_CATEGORY_PCM)) {
        return (flag == 'b') ? get_color_pair(5, -1) : get_color_pair(3, -1);
    }
    if (is_file_category(filename, FILE_CATEGORY_IMAGE)) {
        return (flag == 'c') ? get_color_pair(5, -1) : get_color_pair(154, -1);
    }
    if (is_file_category(filename, FILE_CATEGORY_SYSTEM)) {
        return (flag == 'd') ? get_color_pair(5, -1) : get_color_pair(197, -1);
    }
    if (is_file_category(filename, FILE_CATEGORY_RAW)) {
        return (flag == 'e') ? get_color_pair(5, -1) : get_color_pair(105, -1);
    }
    if (is_file_category(filename, FILE_CATEGORY_ARCHIVE)) {
        return (flag == 'f') ? get_color_pair(5, -1) : get_color_pair(136, -1);
    }
    if (is_file_category(filename, FILE_CATEGORY_AUDIO)) {
        return (flag == 'g') ? get_color_pair(5, -1) : get_color_pair(111, -1);
    }
    if (is_file_category(filename, FILE_CATEGORY_VIDEO)) {
        return (flag == 'h') ? get_color_pair(5, -1) : get_color_pair(42, -1);
    }
    return get_color_pair(15, -1);
}

int get_analyzer_file_color(const char *filename, int is_dir) {
    return get_text_module_file_color(filename, is_dir, ' ');
}
