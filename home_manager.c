#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define HOME_MANAGER_DIR "home-manager"
#define CONFIG_DIR ".config"

// пиздец
int copy_file(const char *source, const char *destination) {
    FILE *src = fopen(source, "rb");
    if (!src) {
        perror("Пошел нахуй, не работает");
        return -1;
    }

    FILE *dest = fopen(destination, "wb");
    if (!dest) {
        fclose(src);
        perror("Пошел нахуй, не работает");
        return -1;
    }

    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dest);
    }

    fclose(src);
    fclose(dest);
    return 0;
}

// Реплейс 
void replace_configurations(const char *home_manager_dir, const char *config_dir) {
    DIR *home_manager_directory = opendir(home_manager_dir);
    if (!home_manager_directory) {
        perror("not found home-manager directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(home_manager_directory)) != NULL) {
        if (entry->d_type == DT_DIR && entry->d_name[0] != '.') { // Игнор скрытых пидоров
            char home_manager_path[512];
            snprintf(home_manager_path, sizeof(home_manager_path), "%s/%s", home_manager_dir, entry->d_name);

            char config_path[512];
            snprintf(config_path, sizeof(config_path), "%s/%s", config_dir, entry->d_name);

            // Чекаем .config
            struct stat st;
            if (stat(config_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                // Копируем файлы из home-manager в .config
                DIR *config_subdir = opendir(home_manager_path);
                if (config_subdir) {
                    struct dirent *file_entry;
                    while ((file_entry = readdir(config_subdir)) != NULL) {
                        if (file_entry->d_type == DT_REG) { // Только обычные файлы
                            char source_file[512];
                            snprintf(source_file, sizeof(source_file), "%s/%s", home_manager_path, file_entry->d_name);

                            char destination_file[512];
                            snprintf(destination_file, sizeof(destination_file), "%s/%s", config_path, file_entry->d_name);

                            printf("Копирование %s в %s\n", source_file, destination_file);
                            copy_file(source_file, destination_file);
                        }
                    }
                    closedir(config_subdir);
                } else {
                    perror("Ошибка открытия поддиректории home-manager");
                }
            }
        }
    }

    closedir(home_manager_directory);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s [update]\n", argv[0]);
        return 1;
    }

    // Получаем путь к домашней директории пользователя
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Не удалось получить домашнюю директорию пользователя.\n");
        return 1;
    }

    char home_manager_dir[512];
    snprintf(home_manager_dir, sizeof(home_manager_dir), "%s/%s", home, HOME_MANAGER_DIR);

    char config_dir[512];
    snprintf(config_dir, sizeof(config_dir), "%s/%s", home, CONFIG_DIR);

    if (strcmp(argv[1], "update") == 0) {
        replace_configurations(home_manager_dir, config_dir);
    } else {
        fprintf(stderr, "Мать ебал: %s\n", argv[1]);
        return 1;
    }

    return 0;
}
