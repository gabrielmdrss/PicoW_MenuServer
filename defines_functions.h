#ifndef DEFINES_FUNCTIONS_H
#define DEFINES_FUNCTIONS_H

int inicialized = 0;

void cabecalho (char *titulo, int x, int y) {
    ssd1306_SetCursor(x, y);
    ssd1306_WriteString(titulo, Font_7x10, White);
    ssd1306_FillRectangle(1, 15, 128, 16, White);
    ssd1306_DrawRectangle(1, 20, 127, 63, White);
}

#endif  /*DEFINES_FUNCTIONS_H*/