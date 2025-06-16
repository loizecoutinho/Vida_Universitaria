#include <allegro5/allegro5.h>              /*funções estruturais iniciais */
#include <allegro5/allegro_font.h>          /* lida com fontes e desenhos de texto */
#include <allegro5/allegro_image.h>         /*manipulação de imagem*/
#include <allegro5/allegro_primitives.h>    /*para desenhar formas como retângulos e círculos */
#include <allegro5/allegro_ttf.h>           /* permite carregamento de fontes ttf e otf; cria textos bonitos */
#include <allegro5/allegro_native_dialog.h> /*para caixa de mensagem */
#include <allegro5/allegro_audio.h>	    /* para áudio*/
#include <allegro5/allegro_acodec.h>	    /*para addons de codecs de audio*/
#include <stdio.h>                          /*entrada e saída*/
#include <string.h>
#include <stdbool.h>                        /*para utilização do tipo bool*/
#include <errno.h>                          /*habilita a var global de erro "errno"*/
#include <time.h>
#include <stdlib.h>

int pontuacao = 0;//pontuação inicial; variavel global
int vida = 3; //quantidade de vida inicial
int aux_velocidade = 0;
int velocidade_blocos = 2;

//Estrutura para um vetor 2D (posição e tamanho)
typedef struct{
    int x, y;
}vetor2d;

//Estrutura para o objeto caindo

typedef struct{
    int livro;
    int papel;
    int comida;
    int lixo;
    int nota_baixa;
    int perdeu_tarefa;
}obj_escolares;

typedef struct{
    int id;
    vetor2d tamanho;
    vetor2d posicao;
    int velo;
    ALLEGRO_COLOR cor; // Alterado de int para ALLEGRO_COLOR
    bool ativo;
}Objeto;

obj_escolares unico_objeto;
Objeto unicoquadrado;

typedef enum {
    PLAYING,    // O jogo está ativo e rodando
    GAME_OVER   // O jogo acabou
} GameState;

GameState game_state;

//funcoes prototipo
int atualizar_quadrado(Objeto *q, ALLEGRO_DISPLAY* disp);
bool checacolisaochao(Objeto *um, int chao_x, int chao_y, int largura_chao,int altura_chao);
bool checacolisaopers(Objeto *um, int persx, int persy, int pers_larg, int pers_alt);
void acelerar(Objeto *unicoquadrado);
void mostrar_lista(obj_escolares *unico_objeto, ALLEGRO_FONT *font, int altura_tela);
void verificar_id(Objeto *unicoquadrado, obj_escolares *unico_obj);
void draw_game_over_screen(ALLEGRO_FONT *font, obj_escolares *unico_objeto, int altura_tela); // Função para desenhar a tela de Game Over
void reset_game_state(void); // Função para reiniciar todas as variáveis do jogo

int main(){
	al_init();
	al_init_native_dialog_addon();
	if(!al_init()){
        al_show_native_message_box(NULL, "Erro","Falha ao inicializar a Biblioteca", "TOP",NULL, ALLEGRO_MESSAGEBOX_WARN);
		return 1;
	}
    //INICIA O TEMPO E DEFINE
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);//atualização de frames; 60 frames por segundo
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();//fila que irá armazenar eventos, como a sequencia de teclas apertadas pelo user;


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
        disp = al_create_display(800, 600);
    }
	if (!disp) {
        //char erro_msg[256];-> não é necessario

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

	//ALLEGRO_FONT* font = al_create_builtin_font();//fonte padrão

	al_register_event_source(queue, al_get_keyboard_event_source()); //registra na fila de evento as ações do user com o teclado;
	al_register_event_source(queue, al_get_display_event_source(disp));//registra na fila de evento as ações no display;
	al_register_event_source(queue, al_get_timer_event_source(timer));//registra na fila de evento as alterações no timer;


	al_set_window_title(disp, "Vida Universitária");


	//adicione as imagens, bitmap
	ALLEGRO_BITMAP *personagem = al_load_bitmap("personagem.png");
	ALLEGRO_BITMAP *background = al_load_bitmap("background.png");
	ALLEGRO_BITMAP *font = al_load_font("PressStart2P-Regular.ttf", 24, 0);

	//verificar se imagem carregou
	if(!personagem){
        fprintf(stderr, "Erro ao criar sprite.\n");
        return 1;
    }
    al_convert_mask_to_alpha(personagem, al_map_rgb(255, 0, 255));//define a cor transparente para a imagem

	//VARIAVEIS

    //variavel booleana (t/f) que define se a personagem esta olhando para a direita ou esquerda
    bool vira_esquerda;
    vira_esquerda = false;//inicializando como false

	//personagem variaveis posicao
	int persx = 100;
	int persy = 380;

	//personagem parte 2, imagens animadas
	int current_frame = 0;
	int total_frames = 4;
	int frame_delay = 5;//alteração de 300para 5
	int frame_count = 0;


    //variaveis para os objetos

	unicoquadrado.ativo = false;
	unico_objeto.comida =0;
    unico_objeto.papel=0;
    unico_objeto.livro=0;
    unico_objeto.lixo=0;
    unico_objeto.nota_baixa= 0;
    unico_objeto.perdeu_tarefa=0;


	int coordenadasimg[4][4] = {
			{0, 0, 283, 983}, //frames 1: x,y,largura,altura. largura � 336 - 0 = 336 , altura � 571 -  0 = 571
			{473, 0, 453, 957}, //frame 2: x = 27, y = 2, largura: 46-27(xf-xi) = 19, altura: 17-2(yf - yi) = 15
			{973, 0, 525, 979}, //frame 3: x = 55, y = 2, largura: 75-55 = 20 ; altura: 17-2 = 15
			{1497, 0, 551, 949} //frame 4, voce tem que aprender a ver o pixel da imagem nos cantos da imagem assim voce consegue o x e y e a largura e altura subtraindo os pontos em x(xf - xi) e y(yf - yi)
	};

	//personagem
    int pers_alt = coordenadasimg[frame_count][4]*0.2;
	int pers_larg = coordenadasimg[frame_count][3]*0.2;
	//chão
	int altura_tela = al_get_display_height(disp);
    int largura_tela = al_get_display_width(disp);
    int altura_chao = 500;
	int chao_x = 0;
	int largura_chao= chao_x + largura_tela;
	int chao_y = altura_chao;

	//
    bool redesenhar;
	bool sair_programa = false;
	ALLEGRO_EVENT event;
    ALLEGRO_KEYBOARD_STATE keyState; //lê o estado do teclado
	al_start_timer(timer);




	//inicie a logica pro jogo
	while(!sair_programa){

	al_wait_for_event(queue, &event); // Espera por um evento


	if(event.type == ALLEGRO_EVENT_TIMER) {
            al_get_keyboard_state(&keyState); // Atualiza o estado do teclado
	}


if(game_state==PLAYING){

            // Movimento do personagem
            if(al_key_down(&keyState, ALLEGRO_KEY_LEFT)) {
                persx -= 5;
                vira_esquerda=true;//quando a tecla esquerda e pressionada, vira_esquerda=true
            }
            if(al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
                persx += 5;
                vira_esquerda=false;//quando a tecla direita e pressionada, vira_esquerda=false
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
                    current_frame = 1;
                }
            }
            if(!al_key_down(&keyState, ALLEGRO_KEY_LEFT) && !al_key_down(&keyState, ALLEGRO_KEY_RIGHT)) {
                current_frame = 0; // Personagem parado
            }

            // Impedir que o personagem saia da tela

            if(persx < 0) persx = 0;

            float largura_redimensionada;//cria uma variavel para compensar, "na largura do sprite" nao sei se e a forma correta de se falar, o fato de que ela foi diminuida
            largura_redimensionada = coordenadasimg[current_frame][2]*0.2;//compensa a largura, tambem com 0.2 pra ficar proporcional perfeitinho

            //meio que muda so que substitui os coordenadasimg... por largura redimensionada para compensar

            if(persx>al_get_display_width(disp)-largura_redimensionada){
            persx = al_get_display_width(disp)-largura_redimensionada;
            }

            // Atualiza os quadrados caindo
            atualizar_quadrado(&unicoquadrado, disp);

            redesenhar = true;

        // Se o evento for fechar a janela
        }else if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE || al_key_down(&keyState, ALLEGRO_KEY_ESCAPE)) {
            sair_programa = true;

        }else if(game_state==GAME_OVER){
            draw_game_over_screen(font, &unico_objeto, altura_tela);
        }// Quando entra em GAME_OVER

        // --- Seção de Desenho (só executa quando necessário) ---
        if(redesenhar) {
            redesenhar = false;

            // Desenha o cenário
            al_draw_bitmap(background, 0,0,NULL);

            // Desenha o quadrado
            if(unicoquadrado.ativo){
                 al_draw_filled_rectangle(
                     unicoquadrado.posicao.x, unicoquadrado.posicao.y,
                     unicoquadrado.posicao.x + unicoquadrado.tamanho.x, unicoquadrado.posicao.y + unicoquadrado.tamanho.y,
                     unicoquadrado.cor
                 );
            }

//comentando a parte que faz o desenho para fazer um teste da personagem virando de lado

            if(vira_esquerda==true){
            al_draw_scaled_bitmap(
                personagem,
                coordenadasimg[current_frame][0], coordenadasimg[current_frame][1],  // igual
                coordenadasimg[current_frame][2], coordenadasimg[current_frame][3],  // igual
                persx + coordenadasimg[current_frame][2]*0.2// mas x tem que ser compensada para que a personagem fiqie posiionada corretamente e se nao tiver esse 0.2 ela teleporta qando vira
                , persy,  // posição y pode continuar a mesma
                -coordenadasimg[current_frame][2]*0.2, coordenadasimg[current_frame][3]*0.2,0);  // negativa a largura pra espelahr, deixa a mesma altura//ago
            }else{
                if(vira_esquerda==false){
            al_draw_scaled_bitmap(
                personagem,
                coordenadasimg[current_frame][0] , coordenadasimg[current_frame][1],
                coordenadasimg[current_frame][2] , coordenadasimg[current_frame][3],
                persx, persy,
                coordenadasimg[current_frame][2]*0.2, coordenadasimg[current_frame][3] *0.2,0);
                //igual o que ja tinha no codigo(para direita)
                }
            }

        }

        al_draw_textf(
            font,
            al_map_rgb(255,255,0), //cor: amarelo
            800/2,                 //centro da tela
            30,                    //posição y
            ALLEGRO_ALIGN_CENTER,  //centralizar
            "VIDA: %d",
            vida
        );

        al_draw_textf(
            font,
            al_map_rgb(255,255,0), //cor: amarelo
            800/2,                 //centro da tela
            20,                    //posição y- perto do topo
            ALLEGRO_ALIGN_CENTER,  //centralizar
            "PONTUAÇÃO: %d",
            pontuacao
        );


	//chão
	printf("Objeto: velo= %d, y=%d, tam_y=%d, Chão: y=%d, alt=%d\n",unicoquadrado.velo, unicoquadrado.posicao.y, unicoquadrado.tamanho.y, chao_y, altura_chao);
        if(checacolisaochao(&unicoquadrado, chao_x, chao_y, largura_chao, altura_chao)){
            unicoquadrado.ativo = false;//pro objeto sumir
            vida--;
            al_draw_textf(
                font,
                al_map_rgb(255,255,0), //cor: amarelo
                800/2,                 //centro da tela
                30,                    //posição y
                ALLEGRO_ALIGN_CENTER,  //centralizar
                "VIDA: %d",
                vida
            );


        }

        //personagem
        else if(checacolisaopers(&unicoquadrado, persx, persy, pers_larg, pers_alt)){
            if(unicoquadrado.ativo){//pro objeto sumir
            unicoquadrado.ativo = false;
            pontuacao++;
            al_draw_textf(
                font,
                al_map_rgb(255,255,0), //cor: amarelo
                800/2,                 //centro da tela
                20,                    //posição y- perto do topo
                ALLEGRO_ALIGN_CENTER,  //centralizar
                "PONTUAÇÃO: %d",
                pontuacao
            );
            verificar_id(&unicoquadrado,&unico_objeto);
            acelerarobj(unicoquadrado);            }
        }
            if(vida<=0){
                game_state=GAME_OVER;
            }

        if(game_state==GAME_OVER){
        draw_game_over_screen(font, &unico_objeto, altura_tela);

            if (al_key_down(&keyState, ALLEGRO_KEY_ENTER)) { // Quando ENTER é pressionado
                    reset_game_state(); // A função de reiniciar é chamada aqui!
                    redesenhar = true;  // Força um redesenho para a nova partida
            }
        }// Quando entra em GAME_OVER



        // Joga tudo que foi desenhado na tela
        al_flip_display();

    }


    // Limpeza de recursos
    al_destroy_bitmap(personagem);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}


int atualizar_quadrado(Objeto *unicoquadrado, ALLEGRO_DISPLAY* disp){
    if(!unicoquadrado->ativo){
        unicoquadrado->id = rand()% 6 + 1;
        unicoquadrado->posicao.x = rand() % (al_get_display_width(disp) - 30);
        unicoquadrado->posicao.y = 0;
        unicoquadrado->cor = al_map_rgb(rand()%256, rand()%256, rand()%256);
        unicoquadrado->velo = 2;
        unicoquadrado->tamanho.x = rand()%16+5;
        unicoquadrado->tamanho.y = rand()%16+5;
        unicoquadrado->ativo = true;
}

    unicoquadrado->posicao.y += velocidade_blocos;

    if(unicoquadrado->posicao.y > al_get_display_height(disp)){
        unicoquadrado->ativo = false;

    }
    return 0;
}

void verificar_id(Objeto *unicoquadrado, obj_escolares *unico_obj){
    if(unicoquadrado->id==1){
        unico_obj->livro++;
    }
    if(unicoquadrado->id==2){
        unico_obj->perdeu_tarefa++;
    }
    if(unicoquadrado->id==3){
        unico_obj->comida++;
    }
    if(unicoquadrado->id==4){
        unico_obj->papel++;
    }
    if(unicoquadrado->id==5){
        unico_obj->lixo++;
    }
    if(unicoquadrado->id==6){
        unico_obj->nota_baixa++;
    }
}

void mostrar_lista(obj_escolares *unico_obj, ALLEGRO_FONT *font, int altura_tela){
    al_draw_textf(font, al_map_rgb(180, 120, 0), 800/2, 280, ALLEGRO_ALIGN_CENTER,
    "CARDAPIO COLETADO");
    al_draw_textf(font, al_map_rgb(180, 120, 0), 800/2, 300 +30, ALLEGRO_ALIGN_CENTER,
    "Livros: %d", unico_obj->livro);
    al_draw_textf(font, al_map_rgb(180, 120, 0), 800/2, 300 +60, ALLEGRO_ALIGN_CENTER,
    "Terefa: %d", unico_obj->papel);
    al_draw_textf(font, al_map_rgb(180, 120, 0), 800/2, 300 +90, ALLEGRO_ALIGN_CENTER,
    "Comida: %d", unico_obj->comida);
    al_draw_textf(font, al_map_rgb(180, 120, 0), 800/2, 300 +120, ALLEGRO_ALIGN_CENTER,
    "Perdeu Tarefa: %d", unico_obj->perdeu_tarefa);
    al_draw_textf(font, al_map_rgb(180, 120, 0), 800/2, 300 +150, ALLEGRO_ALIGN_CENTER,
    "Nota Baixas: %d", unico_obj->nota_baixa);

}


bool checacolisaochao(Objeto *um, int chao_x, int chao_y, int largura_chao, int altura_chao){
    //checa se há colisão no eixo x;
    /*bool colisaoX = um->posicao.x + um->tamanho.x >= chao_x &&
                    chao_x + largura_chao >= um->posicao.x;*/

    //checa se há colisão no eixo y:
    bool colisaoY = um->posicao.y + um->tamanho.y >= chao_y;
    //A colisão só ocorre se houver em ambos os eixos:
    return /*colisaoX &&*/ colisaoY;
}

bool checacolisaopers(Objeto *um, int persx, int persy, int pers_larg, int pers_alt){
    //checa se há colisão no eixo x;
    bool colisaoX = um->posicao.x + um->tamanho.x >= persx &&
                    persx + pers_larg * 0.3 >= um->posicao.x;

    //checa se há colisão no eixo y:
    bool colisaoY = um->posicao.y + um->tamanho.y >= persy &&
                    persy + pers_alt >= um->posicao.y;
    //A colisão só ocorre se houver em ambos os eixos:
    return colisaoX && colisaoY;
}

void acelerarobj(Objeto *unicoquadrado){
        aux_velocidade++;
        if(aux_velocidade>20){
            velocidade_blocos += 2;
            aux_velocidade = 0;
        }
}

void draw_game_over_screen(ALLEGRO_FONT *font, obj_escolares *unico_objeto, int altura_tela){
     al_clear_to_color(al_map_rgb(0,0,0));
                al_draw_text(
                    font,
                    al_map_rgb(255, 0, 0),
                     800/ 2,
                    altura_tela - 500,
                    ALLEGRO_ALIGN_CENTER,
                    "GAME OVER");
    mostrar_lista(unico_objeto, font, altura_tela);
} // Função para desenhar a tela de Game Over

void reset_game_state(void){
    pontuacao = 0;
    vida =3;
    unico_objeto.livro =0;
    unico_objeto.papel=0;
    unico_objeto.comida=0;
    unico_objeto.lixo=0;
    unico_objeto.nota_baixa=0;
    unico_objeto.perdeu_tarefa=0;
    unicoquadrado.ativo=false;
    game_state=PLAYING;
    aux_velocidade = 0;
} // Função para reiniciar todas as variáveis do jogo
