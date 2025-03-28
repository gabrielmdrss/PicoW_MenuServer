#ifndef ICONS_H
#define ICONS_H

/******************************************************************************
 * @file    icons.h
 * @brief   Arquivo contendo definições e arrays para a
 *          visualização do menu.
 *
 * @authors  Gabriel Domingos de Medeiros
 * @date    Fevereiro 2025
 * @version 1.0.0
 *
 * @note    Este arquivo inclui as definições e constantes necessárias
 *          para a visualização do display ssd1306.
 ******************************************************************************/


// --------------------- Variáveis de Seleção de Menu e Itens ---------------------

/**
 * @brief Definições para configuração de itens do menu.
 *
 * Essas variáveis e constantes gerenciam o número de itens no menu, os nomes dos itens
 * e o processo de seleção dentro do menu. A navegação na tela e o gerenciamento do cursor
 * também são tratados através dessas variáveis.
 */
#define NUM_ITEMS  4           ///< Número total de itens no menu, também usado para o número de capturas de tela e códigos QR.
#define MAX_ITEM_LENGTH  20    ///< Comprimento máximo de caracteres permitido para cada nome de item.

int item_selected = 0;         ///< Item atual selecionado no menu.
int item_sel_previous;         ///< Índice do item anterior, usado para exibir o item antes do selecionado.
int item_sel_next;             ///< Índice do próximo item, usado para exibir o item após o selecionado.
int current_screen = 0;        ///< Indica o índice da tela atual sendo exibida.
int cursor = 0;                ///< Posição do cursor no menu para navegação.
int button_enter_clicked = 0;  ///< só executa ação quando o botão ENTER é clicado, e espera até outro clique
int up_clicked = 0;            ///< só executa ação quando o botão é clicado, e espera até outro clique
int down_clicked = 0;          ///< mesmo que acima


// ---------------------- Variáveis de Ícones Bitmap -----------------------

/**
 * @brief Dados do bitmap para o ícone da Nuvem.
 *
 * Este array contém os dados de pixels para o ícone da opção de envio para nuvem, representados em formato hexadecimal.
 * Os dados serão usados para exibir o ícone na tela da aplicação com o display SSD1306.
 */
const unsigned char bitmap_icon_cloud [] = {
	0x00, 0x00, 0x01, 0xc0, 0x0e, 0x20, 0x10, 0x10, 0x10, 0x10, 0x60, 0x58, 0x88, 0x04, 0x90, 0x22,
  0x40, 0x02, 0x3f, 0xfc, 0x00, 0x90, 0x28, 0x14, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/**
 * @brief Dados do bitmap para o ícone da Engrenagem.
 *
 * Este array contém os dados de pixels para o ícone da opção de Setup de Inicialização, representados em formato hexadecimal.
 * Os dados serão usados para exibir o ícone na tela da aplicação com o display SSD1306.
 */
const unsigned char bitmap_icon_setup [] = {
	0x31, 0x8c, 0x5a, 0x5a, 0x6e, 0x76, 0x30, 0x0c, 0x13, 0xc8, 0x17, 0xe8, 0x76, 0x6e, 0x84, 0x21, 
	0x74, 0x2e, 0x16, 0x68, 0x17, 0xe8, 0x30, 0x0c, 0x6e, 0x76, 0x5a, 0x5a, 0x32, 0x4c, 0x01, 0x80
};

/**
 * @brief Dados do bitmap para o ícone da Rede.
 *
 * Este array contém os dados de pixels para o ícone da opção de Informações de Rede, representados em formato hexadecimal.
 * Os dados serão usados para exibir o ícone na tela da aplicação com o display SSD1306.
 */
const unsigned char bitmap_icon_Network [] = {
  0x00, 0x00, 0x3f, 0xe0, 0x40, 0x10, 0x9f, 0xc8, 0x20, 0x20, 0x0f, 0x80, 0x10, 0x4a, 0x07, 0x11,
  0x08, 0x95, 0x02, 0x00, 0x00, 0x04, 0x00, 0x15, 0x00, 0x11, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00
};

/**
 * @brief Dados do bitmap para o ícone do auto falante.
 *
 * Este array contém os dados de pixels para o ícone da opção de controlar frequência do buzzer, representados em formato hexadecimal.
 * Os dados serão usados para exibir o ícone na tela da aplicação com o display SSD1306.
 */
const unsigned char bitmap_icon_speaker [] = {
	0x00, 0x00, 0x00, 0x10, 0x03, 0x08, 0x07, 0x24, 0x0d, 0x12, 0x7b, 0x4a, 0x47, 0x2a, 0x7f, 0x2a, 
	0x7f, 0x2a, 0x7f, 0x2a, 0x7f, 0x4a, 0x0f, 0x12, 0x07, 0x24, 0x03, 0x08, 0x00, 0x10, 0x00, 0x00
};

/**
 * @brief Dados do bitmap para o ícone de fundo da barra de rolagem.
 *
 * Este array contém os dados de pixels para o ícone de fundo da barra de rolagem, representados em formato hexadecimal.
 * Os dados serão usados para exibir o fundo de uma barra de rolagem na tela na aplicação.
 */
const unsigned char bitmap_scrollbar_background [] = {
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
  0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00
};

/**
 * @brief Array de ponteiros para ícones bitmap.
 *
 * Este array contém ponteiros para diferentes ícones bitmap. Ele é usado para acessar os dados dos ícones para várias
 * exibições de tela dentro da aplicação.
 */
const unsigned char* bitmap_icons[4] = {
  bitmap_icon_cloud,        // Ponteiro para o ícone da Nuvem.
  bitmap_icon_setup,        // Ponteiro para o ícone da Engrenagem.
  bitmap_icon_speaker,      // Ponteiro para o ícone do Auto falante.
  bitmap_icon_Network       // Ponteiro para o ícone do Wi-Fi.
};

/**
 * @brief Dados do bitmap para a caixa do item selecionado
 *
 * Este array contém ponteiros para diferentes ícones bitmap. Ele é usado para acessar os dados dos ícones para várias
 * exibições de tela dentro da aplicação.
 */
const unsigned char bitmap_item_sel_outline [] = {
  0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30,
  0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0,
  0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0
};

/**
 * @brief Array dos títulos dos ícones bitmap.
 *
 * Este array contém ponteiros para diferentes ícones bitmap. Ele é usado para acessar os dados dos ícones para várias
 * exibições de tela dentro da aplicação.
 */
char menu_items [NUM_ITEMS] [MAX_ITEM_LENGTH] = {  // array com nomes dos itens
  {"Cloud"},
  {"System Setup"},
  {"Buzzer"},
  {"Network Info"}
 };

#endif /*ICONS_H*/