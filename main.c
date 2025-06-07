#include <allegro5/allegro5.h>              /*funções estruturais iniciais */
#include <allegro5/allegro_font.h>          /* lida com fontes e desenhos de texto */
#include <allegro5/allegro_image.h>         /*manipulação de imagem*/
#include <allegro5/allegro_primitives.h>    /*para desenhar formas como retângulos e círculos */
#include <allegro5/allegro_ttf.h>           /* permite carregamento de fontes ttf e otf; cria textos bonitos */
#include <allegro5/allegro_native_dialog.h> /*para caixa de mensagem */
#include <stdio.h>                          /*entrada e saída*/
#include <string.h>
#include <errno.h>                          /*habilita a var global de erro "errno"*/


typedef struct{
    int tamanho;
    int velo;
    int x, y;
    ALLEGRO_COLOR cor; // Alterado de int para ALLEGRO_COLOR
    bool ativo;
} quadrado;

//funcoes prototipo
int atualizar_quadrado(quadrado *q, ALLEGRO_DISPLAY* disp);

int main(){
	al_init();
	al_init_native_dialog_addon();
	if(!al_init()){
        al_show_native_message_box(NULL, "Erro","Falha ao inicializar a Biblioteca", "TOP",NULL, ALLEGRO_MESSAGEBOX_WARN);
		return 1;
	}
    //INICIA O TEMPO E DEFINE
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);//atualização de frames; 60 frames por segundo
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();


	ALLEGRO_DISPLAY* disp = al_create_display(800, 600);
  	if (!disp) {
        al_show_native_message_box(NULL,
            "Erro",
            "Erro ao iniciar o modo gráfico",
            "Não foi possível criar a janela. Tentando modo de software.",
            NULL, ALLEGRO_MESSAGEBOX_WARN);

        // Define a flag para forçar o uso de um renderizador de software
        al_set_new_display_flags(1);

        // Tenta criar a janela novamente com a nova flag
        disp = al_create_display(320, 200);
    }
	if (!disp) {
        char erro_msg[256];

        al_show_native_message_box(NULL,
            "Erro Crítico",
            "Impossível iniciar a Biblioteca","full",
            NULL, ALLEGRO_MESSAGEBOX_ERROR);

        exit(1); // Encerra o programa
    }

    //carrega as funções de entrada
    al_install_keyboard(); //carrega o teclado
	al_init_image_addon(); //carregar imagens .bmp
	al_init_primitives_addon(); //desenhar formas
	al_init_font_addon();//inicia as fontes de texto
	al_init_ttf_addon();//pode usar fontes ttf

	ALLEGRO_FONT* font = al_create_builtin_font();//fonte padrão

	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(disp));
	al_register_event_source(queue, al_get_timer_event_source(timer));


	al_set_window_title(disp, "Vida Universitária");


	//adicione as imagens, bitmap
	ALLEGRO_BITMAP *personagem = al_load_bitmap("personagem.bmp");
	ALLEGRO_BITMAP *background = al_load_bitmap("background.png");

	//verificar se imagem carregou
	if(!personagem){
        fprintf(stderr, "Erro ao criar sprite.\n");
        return 1;
    }
    al_convert_mask_to_alpha(personagem, al_map_rgb(255, 0, 255));//define a cor transparente para a imagem

	//VARIAVEIS

	//personagem variaveis posicao
	int persx = 100;
	int persy =380;
	//chão
	int altura_tela = al_get_display_height(disp);
	int altura_chao = altura_tela - persy;
	//variaveis do texto
	int score_x = 300;
	int score_y = 150;
	int scr_result = 0;

	//personagem parte 2, imagens animadas
	int current_frame = 0;
	int total_frames = 3;
	int frame_delay = 5;//alteração de 300para 5
	int frame_count = 0;


	//variaveis para os objetos
	quadrado unicoquadrado;
	unicoquadrado.ativo = false;

	int coordenadasimg[3][4] = {
			{0, 0, 336, 571}, //frames 1: x,y,largura,altura. largura � 336 - 0 = 336 , altura � 571 -  0 = 571
			{349, 3, 313, 563}, //frame 2: x = 27, y = 2, largura: 46-27(xf-xi) = 19, altura: 17-2(yf - yi) = 15
			{677, 7, 299, 556}, //frame 3: x = 55, y = 2, largura: 75-55 = 20 ; altura: 17-2 = 15
			//{80, 2, 19, 15} //frame 4, voce tem que aprender a ver o pixel da imagem nos cantos da imagem assim voce consegue o x e y e a largura e altura subtraindo os pontos em x(xf - xi) e y(yf - yi)
	};

	//
	bool redesenhar = true;
	bool sair_programa = false;
	ALLEGRO_EVENT event;
    ALLEGRO_KEYBOARD_STATE keyState; //lê o estado do teclado
	al_start_timer(timer);

	//inicie a logica pro jogo
	while(!sair_programa){
		al_wait_for_event(queue, &event); // Espera por um evento

        if(event.type == ALLEGRO_EVENT_TIMER) {
            al_get_keyboard_state(&keyState); // Atualiza o estado do teclado

            // Movimento do personagem
            if(al_key_down(&keyState, ALLEGRO_KEY_LEFT)) {
                persx -= 5;
            }
            if(al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
                persx += 5;
            }
            if(al_key_down(&keyState, ALLEGRO_KEY_ESCAPE)){
                sair_programa = true;
            }

			//Animação do personagem
            frame_count++;
            if(frame_count >= frame_delay) {
                frame_count = 0;
                current_frame++;
                if(current_frame >= total_frames) {
                    current_frame = 0;
                }
            }
            if(!al_key_down(&keyState, ALLEGRO_KEY_LEFT) && !al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
                current_frame = 0; // Personagem parado
            }

            // Impedir que o personagem saia da tela
            if(persx < 0) persx = 0;
            if(persx > al_get_display_width(disp) - coordenadasimg[current_frame][2]){
                persx = al_get_display_width(disp) - coordenadasimg[current_frame][2];
            }

            // Atualiza os quadrados caindo
            atualizar_quadrado(&unicoquadrado, disp);

            redesenhar = true;
        }
        // Se o evento for fechar a janela
        else if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            sair_programa = true;
        }
        // --- Seção de Desenho (só executa quando necessário) ---
        if(redesenhar && al_is_event_queue_empty(queue)) {
            redesenhar = false;




            // Desenha o cenário
            al_draw_bitmap(background, 0,0,NULL);
           // al_draw_filled_rectangle(0, altura_tela - altura_chao +10 , 800, altura_tela, al_map_rgba(50,50,30,0));
            al_draw_filled_circle(100, 100, 50, al_map_rgb(255,255,0));

            // Desenha o quadrado
            if(unicoquadrado.ativo){
                 al_draw_filled_rectangle(
                     unicoquadrado.x, unicoquadrado.y,
                     unicoquadrado.x + unicoquadrado.tamanho, unicoquadrado.y + unicoquadrado.tamanho,
                     unicoquadrado.cor
                 );
            }

            // Desenha a região correta do personagem (spritesheet)
            al_draw_scaled_bitmap(
                personagem,
                coordenadasimg[current_frame][0] , coordenadasimg[current_frame][1],
                coordenadasimg[current_frame][2] , coordenadasimg[current_frame][3],
                persx, persy,
                coordenadasimg[current_frame][2]*0.2, coordenadasimg[current_frame][3] *0.2,
                0);

             //fazendo o textos na tela
             al_draw_textf(font, al_map_rgb(255,255,255), score_x, score_y, ALLEGRO_ALIGN_CENTER, "Score: %d", scr_result);
             scr_result++;

            // Joga tudo que foi desenhado na tela
            al_flip_display();
            //substitui a tela anteiro
            al_clear_to_color(al_map_rgb(0,0,0));
        }
    }

    // Limpeza de recursos
    al_destroy_bitmap(personagem);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}


int atualizar_quadrado(quadrado *q, ALLEGRO_DISPLAY* disp){
    if(!q->ativo){
        q->x = rand() % (al_get_display_width(disp) - 30);
        q->y = 0;
        q->cor = al_map_rgb(rand()%256, rand()%256, rand()%256);
        q->velo = 2 + (rand()%3);
        q->tamanho = rand()%16+5;
        q->ativo = true;
    }

    q->y += q->velo;

    if(q->y > al_get_display_height(disp)){
        q->ativo = false;
        return 0;
    }

    return 1;
}
