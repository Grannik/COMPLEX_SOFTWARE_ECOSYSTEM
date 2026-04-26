#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/socket.h>
#include <magic.h>
#include <iconv.h>
#include <uchardet/uchardet.h>
#include <archive.h>
#include <archive_entry.h>
#include <libavformat/avformat.h>
#include <libexif/exif-data.h>
#include <sys/socket.h>

int mod_01_run(const char *filename);
int mod_02_run(const char *filename);
int mod_03_run(const char *filename);
int mod_04_run(const char *filename);
int mod_05_run(const char *filename);
int mod_06_run(const char *filename);
int mod_07_run(const char *filename);
int mod_08_run(const char *filename);
int mod_09_run(const char *filename);
int mod_10_run(const char *filename);
int mod_11_run(const char *filename);

#define BUFFER_SIZE 4096
#define MAX_FILE_SIZE (1024 * 1024 * 1024)
#define UNUSED __attribute__((unused))

int mod_01_run(const char *filename) {
    char log_msg[1024];
    struct stat st;
    if (stat(filename, &st) != 0) {
        snprintf(log_msg, sizeof(log_msg), " Main: " "Cannot stat file: %s", strerror(errno));
        printf("%s\n", log_msg);
        return 1;
    }

    int is_regular = S_ISREG(st.st_mode);
    snprintf(log_msg, sizeof(log_msg)," Main: " "File is %s", is_regular ? "regular file" : "not a regular file");
    printf("%s\n", log_msg);
    return 0;
}

int mod_02_run(const char *filename) {
    char log_msg[1024];

    FILE *file = fopen(filename, "rb");
    if (!file) {
        snprintf(log_msg, sizeof(log_msg), " Library text_analyzer: " "File type not recognized: %s", strerror(errno));
        printf("%s\n", log_msg);
        return 1;
    }

    unsigned char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        snprintf(log_msg, sizeof(log_msg), " Library text_analyzer: " "Memory allocation failed");
        printf("%s\n", log_msg);
        fclose(file);
        return 1;
    }

    size_t read_bytes = fread(buffer, 1, BUFFER_SIZE, file);
    fclose(file);

    if (read_bytes == 0) {
        snprintf(log_msg, sizeof(log_msg), " Library text_analyzer: " "File type not recognized: Empty file");
        printf("%s\n", log_msg);
        free(buffer);
        return 1;
    }

    int has_ansi = 0;
    for (size_t i = 0; i < read_bytes - 1; i++) {
        if (buffer[i] == '\x1B' && buffer[i + 1] == '[') {
            has_ansi = 1;
            break;
        }
    }

    int is_json = 0;
    for (size_t i = 0; i < read_bytes; i++) {
        if (buffer[i] == ' ' || buffer[i] == '\t' || buffer[i] == '\n' || buffer[i] == '\r') continue;
        if (buffer[i] == '{' || buffer[i] == '[') {
            is_json = 1;
        }
        break;
    }

    int is_csv = 0;
    int comma_count = 0;
    for (size_t i = 0; i < read_bytes; i++) {
        if (buffer[i] == ',' || buffer[i] == ';') {
            comma_count++;
            if (comma_count >= 3) {
                is_csv = 1;
                break;
            }
        }
    }

    if (has_ansi) {
        snprintf(log_msg, sizeof(log_msg), " Library text_analyzer: " "Text file with ANSI escape sequences (likely terminal log)");
    } else if (is_json) {
        snprintf(log_msg, sizeof(log_msg), " Library text_analyzer: " "Text file, likely JSON format");
    } else if (is_csv) {
        snprintf(log_msg, sizeof(log_msg), " Library text_analyzer: " "Text file, likely CSV format");
    } else {
        snprintf(log_msg, sizeof(log_msg), " Library text_analyzer: " "Text file, unknown structure (possibly plain text or log)");
    }
    printf("%s\n", log_msg);
    free(buffer);
    return 0;
}

int mod_03_run(const char *filename) {
    char log_msg[1024];

    struct stat st;
    if (stat(filename, &st) != 0) {
        snprintf(log_msg, sizeof(log_msg), " Library libstat: " "File type not recognized: %s", strerror(errno));
        printf("%s\n", log_msg);
        return 1;
    }

    const char *type;
    char extra_info[512] = "";
    if (S_ISREG(st.st_mode)) {
        type = "regular file";
    } else if (S_ISDIR(st.st_mode)) {
        type = "directory";
    } else if (S_ISCHR(st.st_mode)) {
        type = "character device";
        snprintf(extra_info, sizeof(extra_info), ", Major: %d, Minor: %d",
                 major(st.st_rdev), minor(st.st_rdev));
    } else if (S_ISBLK(st.st_mode)) {
        type = "block device";
        snprintf(extra_info, sizeof(extra_info), ", Major: %d, Minor: %d",
                 major(st.st_rdev), minor(st.st_rdev));
    } else if (S_ISFIFO(st.st_mode)) {
        type = "FIFO (named pipe)";
    } else if (S_ISSOCK(st.st_mode)) {
        type = "socket";
    } else if (S_ISLNK(st.st_mode)) {
        type = "symbolic link";
        char link_target[256];
        ssize_t len = readlink(filename, link_target, sizeof(link_target) - 1);
        if (len != -1) {
            link_target[len] = '\0';
            snprintf(extra_info, sizeof(extra_info), ", Points to: %s", link_target);
        } else {
            snprintf(extra_info, sizeof(extra_info), ", Error reading link: %s", strerror(errno));
        }
    } else {
        type = "unknown";
    }

    if (strncmp(filename, "/proc/", 6) == 0) {
        type = "procfs pseudo-file";
    } else if (strncmp(filename, "/sys/", 5) == 0) {
        type = "sysfs pseudo-file";
    }

    snprintf(log_msg, sizeof(log_msg), " Library libstat: " "Size: %ld bytes, Permissions: %o, Type: %s%s",
             (long)st.st_size, st.st_mode & 0777, type, extra_info);
    printf("%s\n", log_msg);
    return 0;
}

int mod_04_run(const char *filename) {
    char log_msg[1024];

    magic_t magic = magic_open(MAGIC_MIME_TYPE | MAGIC_ERROR | MAGIC_NO_CHECK_CDF);
    if (!magic) {
        snprintf(log_msg, sizeof(log_msg), " Library libmagic: " "File type not recognized: Error initializing libmagic");
        printf("%s\n", log_msg);
        return 1;
    }

    if (magic_load(magic, NULL) != 0) {
        snprintf(log_msg, sizeof(log_msg), " Library libmagic: " "File type not recognized: Error loading magic database: %s",
                 magic_error(magic));
        printf("%s\n", log_msg);
        magic_close(magic);
        return 1;
    }

    const char *mime = magic_file(magic, filename);
    if (!mime || strlen(mime) == 0 || strstr(mime, "\xFF") || strstr(mime, "\x00")) {
        mime = "text/plain";
        snprintf(log_msg, sizeof(log_msg), " Library libmagic: " "Invalid MIME, assuming text/plain");
        printf("%s\n", log_msg);
    }

    magic_setflags(magic, 0);
    const char *desc = magic_file(magic, filename);
    snprintf(log_msg, sizeof(log_msg), " Library libmagic: " "MIME: %s, Description: %s",
             mime, desc ? desc : "Unknown");
    printf("%s\n", log_msg);
    magic_close(magic);
    return 0;
}

int mod_05_run(const char *filename) {
    char log_msg[1024];

    FILE *file = fopen(filename, "rb");
    if (!file) {
        snprintf(log_msg, sizeof(log_msg),  " Library iconv: " "File type not recognized: %s", strerror(errno));
        printf("%s\n", log_msg);
        return 1;
    }

    unsigned char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        snprintf(log_msg, sizeof(log_msg),  " Library iconv: " "Memory allocation failed");
        printf("%s\n", log_msg);
        fclose(file);
        return 1;
    }

    size_t read_bytes = fread(buffer, 1, BUFFER_SIZE, file);
    fclose(file);

    if (read_bytes == 0) {
        snprintf(log_msg, sizeof(log_msg), " Library iconv: " "File type not recognized: Empty file");
        printf("%s\n", log_msg);
        free(buffer);
        return 1;
    }

    iconv_t cd = iconv_open("UTF-8", "UTF-8");
    if (cd == (iconv_t)-1) {
        snprintf(log_msg, sizeof(log_msg), " Library iconv: " "Error initializing iconv");
        printf("%s\n", log_msg);
        free(buffer);
        return 1;
    }

    char *inbuf = (char *)buffer;
    size_t inbytesleft = read_bytes;
    char *outbuf = malloc(BUFFER_SIZE);
    if (!outbuf) {
        snprintf(log_msg, sizeof(log_msg), " Library iconv: " "Memory allocation failed");
        printf("%s\n", log_msg);
        iconv_close(cd);
        free(buffer);
        return 1;
    }
    char *outptr = outbuf;
    size_t outbytesleft = BUFFER_SIZE;

    size_t result = iconv(cd, &inbuf, &inbytesleft, &outptr, &outbytesleft);
    if (result == (size_t)-1) {
        snprintf(log_msg, sizeof(log_msg), " Library iconv: " "Not UTF-8, possible binary or other encoding");
    } else {
        snprintf(log_msg, sizeof(log_msg), " Library iconv: " "Compatible with UTF-8");
    }
    printf("%s\n", log_msg);
    iconv_close(cd);
    free(outbuf);
    free(buffer);
    return 0;
}

int mod_06_run(const char *filename) {
    char log_msg[1024];

    FILE *file = fopen(filename, "rb");
    if (!file) {
        snprintf(log_msg, sizeof(log_msg), " Library uchardet: " "File type not recognized: %s" , strerror(errno));
        printf("%s\n", log_msg);
        return 1;
    }

    unsigned char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        snprintf(log_msg, sizeof(log_msg), " Library uchardet: " "Memory allocation failed");
        printf("%s\n", log_msg);
        fclose(file);
        return 1;
    }

    size_t read_bytes = fread(buffer, 1, BUFFER_SIZE, file);
    fclose(file);

    if (read_bytes == 0) {
        snprintf(log_msg, sizeof(log_msg), " Library uchardet: " "File type not recognized: Empty file");
        printf("%s\n", log_msg);
        free(buffer);
        return 1;
    }

    uchardet_t handle = uchardet_new();
    if (!handle) {
        snprintf(log_msg, sizeof(log_msg), " Library uchardet: " "Memory allocation failed");
        printf("%s\n", log_msg);
        free(buffer);
        return 1;
    }

    if (uchardet_handle_data(handle, (const char *)buffer, read_bytes) != 0) {
        snprintf(log_msg, sizeof(log_msg), " Library uchardet: " "Error processing data");
        printf("%s\n", log_msg);
        uchardet_delete(handle);
        free(buffer);
        return 1;
    }

    uchardet_data_end(handle);
    const char *encoding = uchardet_get_charset(handle);
    if (encoding && strlen(encoding) > 0) {
        snprintf(log_msg, sizeof(log_msg), " Library uchardet: " "Detected encoding: %s" , encoding);
    } else {
        snprintf(log_msg, sizeof(log_msg), " Library uchardet: " "Unknown encoding");
    }
    printf("%s\n", log_msg);
    uchardet_delete(handle);
    free(buffer);
    return 0;
}

int mod_07_run(const char *filename) {
    char log_msg[1024];

    struct archive *a = archive_read_new();
    if (!a) {
        snprintf(log_msg, sizeof(log_msg), " Library libarchive: " "Error initializing libarchive");
        printf("%s\n", log_msg);
        return 1;
    }

    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if (archive_read_open_filename(a, filename, 10240) != ARCHIVE_OK) {
        snprintf(log_msg, sizeof(log_msg), " Library libarchive: " "Not an archive: %s",
                 archive_error_string(a));
        printf("%s\n", log_msg);
        archive_read_free(a);
        return 1;
    }

    struct archive_entry *entry;
    int entries_read = 0;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        entries_read++;
    }

    if (entries_read > 0) {
        snprintf(log_msg, sizeof(log_msg), " Library libarchive: " "Archive format: %s, Entries: %d",
                 archive_format_name(a), entries_read);
    } else {
        snprintf(log_msg, sizeof(log_msg), " Library libarchive: " "Corrupted or empty archive");
    }
    printf("%s\n", log_msg);
    archive_read_close(a);
    archive_read_free(a);
    return 0;
}

int mod_08_run(const char *filename) {
    char log_msg[1024];
av_log_set_level(AV_LOG_QUIET);
    AVFormatContext *fmt_ctx = NULL;
    if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0) {
        snprintf(log_msg, sizeof(log_msg), " Library libavformat: " "Error opening file");
        printf("%s\n", log_msg);
        return 1;
    }

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        snprintf(log_msg, sizeof(log_msg), " Library libavformat: " "Error finding stream info");
        printf("%s\n", log_msg);
        avformat_close_input(&fmt_ctx);
        return 1;
    }

    snprintf(log_msg, sizeof(log_msg), " Library libavformat: " "Media format: %s, Duration: %ld seconds, Streams: %d",
             fmt_ctx->iformat->name, fmt_ctx->duration / AV_TIME_BASE, fmt_ctx->nb_streams);
    printf("%s\n", log_msg);
    avformat_close_input(&fmt_ctx);
    return 0;
}

typedef struct {
    const char *name;
    const unsigned char *signature;
    size_t sig_len;
    const char *mime_type;
    int offset;
} Signature;

static const Signature signatures[] = {
    {"PNG", (const unsigned char *)"\x89PNG\r\n\x1A\n", 8, "image/png", 0},
    {"JPEG", (const unsigned char *)"\xFF\xD8\xFF", 3, "image/jpeg", 0},
    {"ELF", (const unsigned char *)"\x7F" "ELF", 4, "application/x-executable", 0},
    {"PDF", (const unsigned char *)"%PDF-", 5, "application/pdf", 0},
    {"GIF", (const unsigned char *)"GIF89a", 6, "image/gif", 0},
    {"GIF", (const unsigned char *)"GIF87a", 6, "image/gif", 0},
    {"BMP", (const unsigned char *)"BM", 2, "image/bmp", 0},
    {"WEBP", (const unsigned char *)"RIFF....WEBP", 12, "image/webp", 0},
    {"MP3", (const unsigned char *)"ID3", 3, "audio/mpeg", 0},
    {"WAV", (const unsigned char *)"RIFF....WAVE", 12, "audio/wav", 0},
    {"FLAC", (const unsigned char *)"fLaC", 4, "audio/flac", 0},
    {"MP4", (const unsigned char *)"\x00\x00\x00\x20\x66\x74\x79\x70\x69\x73\x6F\x6D", 12, "video/mp4", 0},
    {"MP4", (const unsigned char *)"\x00\x00\x00\x18\x66\x74\x79\x70\x6D\x70\x34\x32", 12, "video/mp4", 0},
    {"MP4", (const unsigned char *)"\x00\x00\x00\x1C\x66\x74\x79\x70\x64\x61\x73\x68", 12, "video/mp4", 0},
    {"MKV", (const unsigned char *)"\x1A\x45\xDF\xA3", 4, "video/x-matroska", 0},
    {"ZIP", (const unsigned char *)"PK\x03\x04", 4, "application/zip", 0},
    {"TAR", (const unsigned char *)"ustar", 5, "application/x-tar", 257},
    {"7Z", (const unsigned char *)"\x37\x7A\xBC\xAF\x27\x1C", 6, "application/x-7z-compressed", 0},
    {"DOCX", (const unsigned char *)"PK\x03\x04", 4, "application/vnd.openxmlformats-officedocument.wordprocessingml.document", 0},
    {"XLSX", (const unsigned char *)"PK\x03\x04", 4, "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", 0},
    {"SQLITE", (const unsigned char *)"SQLite format 3", 15, "application/x-sqlite3", 0},
    {"EXE", (const unsigned char *)"MZ", 2, "application/x-msdownload", 0},
    {"ISO", (const unsigned char *)"\x43\x44\x30\x30\x31", 5, "application/x-iso9660-image", 32769},
    {"OGG", (const unsigned char *)"OggS", 4, "audio/ogg", 0},
    {"AVI", (const unsigned char *)"RIFF....AVI ", 12, "video/x-msvideo", 0},
    {"RAR", (const unsigned char *)"Rar!\x1A\x07\x00", 7, "application/x-rar-compressed", 0},
    {"EPUB", (const unsigned char *)"PK\x03\x04", 4, "application/epub+zip", 0},
    {"ODT", (const unsigned char *)"PK\x03\x04", 4, "application/vnd.oasis.opendocument.text", 0},
    {"SIXEL", (const unsigned char *)"\x1B\x50\x71", 3, "image/sixel", 0},
    {NULL, NULL, 0, NULL, 0}
};

int mod_09_run(const char *filename) {
    char log_msg[1024];

    FILE *file = fopen(filename, "rb");
    if (!file) {
        snprintf(log_msg, sizeof(log_msg), " Library custom_signature: " "File type not recognized: %s", strerror(errno));
        printf("%s\n", log_msg);
        return 1;
    }

    int found = 0;
    for (int i = 0; signatures[i].name; i++) {
        fseek(file, signatures[i].offset, SEEK_SET);
        unsigned char *buffer = malloc(signatures[i].sig_len);
        if (!buffer) {
            snprintf(log_msg, sizeof(log_msg), " Library custom_signature: " "Memory allocation failed");
            printf("%s\n", log_msg);
            fclose(file);
            return 1;
        }

        size_t read_bytes = fread(buffer, 1, signatures[i].sig_len, file);
        if (read_bytes >= signatures[i].sig_len &&
            memcmp(buffer, signatures[i].signature, signatures[i].sig_len) == 0) {
            snprintf(log_msg, sizeof(log_msg), " Library custom_signature: " "Signature match: %s, MIME: %s",
                     signatures[i].name, signatures[i].mime_type);
            printf("%s\n", log_msg);
            found = 1;
            free(buffer);
            break;
        }
        free(buffer);
    }
    fclose(file);

    if (!found) {
        snprintf(log_msg, sizeof(log_msg), " Library custom_signature: " "No known signature found");
        printf("%s\n", log_msg);
    }
    return 0;
}

int mod_10_run(const char *filename) {
    char log_msg[1024];

    FILE *file = fopen(filename, "rb");
    if (!file) {
        snprintf(log_msg, sizeof(log_msg), " Library libc: " "File type not recognized: %s", strerror(errno));
        printf("%s\n", log_msg);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size > MAX_FILE_SIZE) {
        snprintf(log_msg, sizeof(log_msg), " Library libc: " "File too large: %ld bytes", size);
        printf("%s\n", log_msg);
        fclose(file);
        return 1;
    }

    unsigned char *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        snprintf(log_msg, sizeof(log_msg), " Library libc: " "Memory allocation failed");
        printf("%s\n", log_msg);
        fclose(file);
        return 1;
    }

    size_t read_bytes = fread(buffer, 1, BUFFER_SIZE, file);
    fclose(file);

    if (read_bytes == 0) {
        snprintf(log_msg, sizeof(log_msg), " Library libc: " "File type not recognized: Empty file");
        printf("%s\n", log_msg);
        free(buffer);
        return 1;
    }

    int is_text = 1;
    for (size_t i = 0; i < read_bytes; i++) {
        if (buffer[i] == 0) {
            is_text = 0;
            break;
        }
        if (buffer[i] < 32 && buffer[i] != '\n' && buffer[i] != '\r' && buffer[i] != '\t' && buffer[i] != '\x1B') {
            is_text = 0;
            break;
        }
    }

    if (is_text) {
        char *text = malloc(read_bytes + 1);
        if (!text) {
            snprintf(log_msg, sizeof(log_msg), " Library libc: " "Memory allocation failed");
            printf("%s\n", log_msg);
            free(buffer);
            return 1;
        }
        memcpy(text, buffer, read_bytes);
        text[read_bytes] = '\0';
        snprintf(log_msg, sizeof(log_msg)," Library libc: " "Size: %ld bytes, First %zu bytes (text): %.100s...",
                 size, read_bytes, text);
        free(text);
    } else {
        snprintf(log_msg, sizeof(log_msg), " Library libc: " "Size: %ld bytes, First %zu bytes (binary)",
                 size, read_bytes);
    }
    printf("%s\n", log_msg);
    free(buffer);
    return 0;
}

int mod_11_run(const char *filename) {
    char log_msg[1024];

    FILE *file = fopen(filename, "rb");
    if (!file) {
        snprintf(log_msg, sizeof(log_msg), " Library libexif: " "File type not recognized: %s", strerror(errno));
        printf("%s\n", log_msg);
        return 1;
    }

    unsigned char buffer[2];
    size_t read_bytes = fread(buffer, 1, 2, file);
    fclose(file);
    if (read_bytes != 2 || buffer[0] != 0xFF || buffer[1] != 0xD8) {
        snprintf(log_msg, sizeof(log_msg), " Library libexif: " "Not a JPEG file");
        printf("%s\n", log_msg);
        return 1;
    }

    ExifData *ed = exif_data_new_from_file(filename);
    if (!ed) {
        snprintf(log_msg, sizeof(log_msg), " Library libexif: " "No EXIF data found");
        printf("%s\n", log_msg);
        return 1;
    }

    char resolution[32] = "Unknown";
    char date_time[32] = "Unknown";
    char camera_model[64] = "Unknown";
    char gps_info[128] = "None";

    ExifEntry *entry_x = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_PIXEL_X_DIMENSION);
    ExifEntry *entry_y = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_PIXEL_Y_DIMENSION);
    if (entry_x && entry_y) {
        char x_str[16], y_str[16];
        exif_entry_get_value(entry_x, x_str, sizeof(x_str));
        exif_entry_get_value(entry_y, y_str, sizeof(y_str));
        snprintf(resolution, sizeof(resolution), "%sx%s", x_str, y_str);
    }

    ExifEntry *entry_date = exif_content_get_entry(ed->ifd[EXIF_IFD_EXIF], EXIF_TAG_DATE_TIME_ORIGINAL);
    if (entry_date) {
        exif_entry_get_value(entry_date, date_time, sizeof(date_time));
    }

    ExifEntry *entry_model = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_MODEL);
    if (entry_model) {
        exif_entry_get_value(entry_model, camera_model, sizeof(camera_model));
    }

    ExifEntry *entry_lat = exif_content_get_entry(ed->ifd[EXIF_IFD_GPS], EXIF_TAG_GPS_LATITUDE);
    ExifEntry *entry_lon = exif_content_get_entry(ed->ifd[EXIF_IFD_GPS], EXIF_TAG_GPS_LONGITUDE);
    if (entry_lat && entry_lon) {
        char lat_str[32], lon_str[32];
        exif_entry_get_value(entry_lat, lat_str, sizeof(lat_str));
        exif_entry_get_value(entry_lon, lon_str, sizeof(lon_str));
        snprintf(gps_info, sizeof(gps_info), "Lat: %s, Lon: %s", lat_str, lon_str);
    }

    snprintf(log_msg, sizeof(log_msg)," Library libexif: " "Resolution: %s, Taken: %s, Camera: %s, GPS: %s",
             resolution, date_time, camera_model, gps_info);
    printf("%s\n", log_msg);

    exif_data_unref(ed);
    return 0;
}
