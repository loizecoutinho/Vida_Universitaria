#include <allegro5/allegro5.h>              /*funções estruturais iniciais */
#include <allegro5/allegro_font.h>          /* lida com fontes e desenhos de texto */
#include <allegro5/allegro_image.h>         /*manipulação de imagem*/
#include <allegro5/allegro_primitives.h>    /*para desenhar formas como retângulos e círculos */
#include <allegro5/allegro_ttf.h>           /* permite carregamento de fontes ttf e otf; cria textos bonitos */
#include <allegro5/allegro_native_dialog.h> /*para caixa de mensagem */
#include <allegro5/allegro_audio.h>	        /* para áudio*/
#include <allegro5/allegro_acodec.h>	    /*para addons de codecs de audio*/
#include <stdio.h>                          /*entrada e saída*/
#include <string.h>
#include <stdbool.h>                        /*para utilização do tipo bool*/
#include <errno.h>                          /*habilita a var global de erro "errno"*/
#include <time.h>
#include <stdlib.h>

#define BGM_FILE "School.ogg" //definindo caminho para o arquivo

int record=0;
int pontuacao = 0;//pontuação inicial; variavel global
int vida = 3; //quantidade de vida inicial
int aux_velocidade = 0;
int velocidade_blocos = 2;
int frame_obj;
//Estrutura para um vetor 2D (posição e tamanho)
typedef struct{
    int x, y;
}vetor2d;
//Estrutura para o objeto caindo
typedef struct{
    int livro;
    int notebook;
    int garrafa;
    int bloco_notas;
    int diario;
    int caderno;
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
    GAME_OVER,   // O jogo acabou
    PAUSE,   //jogo pausado
    MENU     //jogo no menu
} GameState;

GameState game_state;

//funcoes prototipo
int atualizar_quadrado(Objeto *unicoquadrado, int coordenadasobjs[][4], ALLEGRO_DISPLAY* disp);
bool checacolisaochao(Objeto *um, int chao_x, int chao_y, int largura_chao,int altura_chao);
bool checacolisaopers(Objeto *um, int persx, int persy, int pers_larg, int pers_alt,int record);
void acelerar(Objeto *unicoquadrado);//acelera os objetos
void mostrar_lista(obj_escolares *unico_objeto, ALLEGRO_FONT *font, ALLEGRO_FONT *font1, int altura_tela);
void verificar_id(Objeto *unicoquadrado, obj_escolares *unico_obj);
void draw_game_over_screen(ALLEGRO_FONT *font, ALLEGRO_FONT *font1, obj_escolares *unico_objeto, int altura_tela); // Função para desenhar a tela de Game Over
void reset_game_state(void); // Função para reiniciar todas as variáveis do jogo

int main(){

    if(!al_init()){
        al_show_native_message_box(NULL, "Erro","Falha ao inicializar a Biblioteca", "TOP",NULL, ALLEGRO_MESSAGEBOX_WARN);
		return 1;
	}

	al_init_native_dialog_addon();

    al_install_audio();//inicializa o addon de audio
    if(!al_install_audio){
        al_show_native_message_box(NULL, "Erro","Falha ao inicializar o áudio", "TOP",NULL, ALLEGRO_MESSAGEBOX_WARN);
		return 1;
    }

    al_init_acodec_addon();//inicializa o addon de codecs para o áudio(habilita para o formato OGG)
    if(!al_init_acodec_addon()){
        al_show_native_message_box(NULL, "Erro","Falha ao inicializar o addon de codecs do áudi", "TOP",NULL, ALLEGRO_MESSAGEBOX_WARN);
		return 1;
    }

    //cria automaticamente o mixer
    al_reserve_samples(16);//reserva de samples, caso seja necessario usar algumas vozes
    if (!al_reserve_samples(16)) {
        fprintf(stderr, "Falha ao reservar samples!\n");
        // Não é um erro crítico para streams
    }

    //carrega as funções de entrada
    al_install_keyboard(); //carrega o teclado
	al_init_image_addon(); //carregar imagens .bmp
	al_init_primitives_addon(); //desenhar formas
	al_init_font_addon();//inicia as fontes de texto
	al_init_ttf_addon();//pode usar fontes ttf

    //INICIA O TEMPO E DEFINE
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);//atualização de frames; 60 frames por segundo
	ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();//fila que irá armazenar eventos, como a sequencia de teclas apertadas pelo user;
    srand(time(NULL));// criar os objetos em lugares diferentes cada inicialização
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
        al_show_native_message_box(NULL,
            "Erro Crítico",
            "Impossível iniciar a Biblioteca","full",
            NULL, ALLEGRO_MESSAGEBOX_ERROR);
        exit(1); // Encerra o programa
    }

	//ALLEGRO_FONT* font = al_create_builtin_font();//fonte padrão
	al_register_event_source(queue, al_get_keyboard_event_source()); //registra na fila de evento as ações do user com o teclado;
	al_register_event_source(queue, al_get_display_event_source(disp));//registra na fila de evento as ações no display;
	al_register_event_source(queue, al_get_timer_event_source(timer));//registra na fila de evento as alterações no timer;
	al_set_window_title(disp, "Vida Universitária");


	//adicione as imagens, bitmap
	ALLEGRO_BITMAP *fundo_menu = al_load_bitmap("fundomenuuni1.png");
	ALLEGRO_BITMAP *personagem = al_load_bitmap("personagem.png");
	ALLEGRO_BITMAP *background = al_load_bitmap("background.png");
	ALLEGRO_BITMAP *objetos1 = al_load_bitmap("objetos.png");
	ALLEGRO_FONT *font = al_load_font("PressStart2P-Regular.ttf", 24, 0);
    ALLEGRO_FONT *font1 = al_load_font("PressStart2P-Regular.ttf", 28, 0);

    ALLEGRO_FONT* fonte_titulo = al_load_ttf_font("PressStart2P-Regular.ttf", 28, 0);//fonte press start 2p para titulo no menu
	ALLEGRO_FONT* fonte_opcoes = al_load_ttf_font("PressStart2P-Regular.ttf", 15, 0);//fonte press start 2p para opções no menu
	ALLEGRO_FONT* fonte_pause = al_load_ttf_font("PressStart2P-Regular.ttf", 15, 0);//fonte press start 2p para opções no menu

    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
    al_convert_mask_to_alpha(personagem, al_map_rgb(255, 0, 255));//define a cor transparente para a imagem
    al_convert_mask_to_alpha(objetos1, al_map_rgb(255, 0, 255));
	//verificar se imagem carregou
	if(!personagem){
        fprintf(stderr, "Erro ao criar sprite.\n");
        return 1;
    }
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
	unico_objeto.livro=0;
	unico_objeto.notebook =0;
    unico_objeto.garrafa=0;
    unico_objeto.bloco_notas=0;
    unico_objeto.diario= 0;
    unico_objeto.caderno=0;
	int coordenadasimg[4][4] = {
			{0, 0, 283, 983}, //frames 1: x,y,largura,altura. largura � 336 - 0 = 336 , altura � 571 -  0 = 571
			{473, 0, 453, 957}, //frame 2: x = 27, y = 2, largura: 46-27(xf-xi) = 19, altura: 17-2(yf - yi) = 15
			{973, 0, 525, 979}, //frame 3: x = 55, y = 2, largura: 75-55 = 20 ; altura: 17-2 = 15
			{1497, 0, 551, 949} //frame 4, voce tem que aprender a ver o pixel da imagem nos cantos da imagem assim voce consegue o x e y e a largura e altura subtraindo os pontos em x(xf - xi) e y(yf - yi)
	};
	int coordenadasobjs[6][4] = {
            {20,10,42,64}, //caderno
            {18,80,49,66}, //bloco de notas
            {9,148,62,65}, //livro
            {23,225,35,65}, //garrafa
            {3,291,107,111}, //notebook
            {10,443,68,87 } //diario
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

    al_create_audio_stream(6, 2048, 44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
    ALLEGRO_AUDIO_STREAM *bgm_stream = al_load_audio_stream(BGM_FILE, 4, 2048); // Buffer size 4 é um bom valor.
    if (!bgm_stream) {
        fprintf(stderr, "Falha ao carregar stream de áudio: %s\n", BGM_FILE);
        // Não é um erro crítico que impeça o jogo de rodar, mas o áudio não funcionará.
    } else {
        if (!al_attach_audio_stream_to_mixer(bgm_stream, al_get_default_mixer())) {
            fprintf(stderr, "Falha ao anexar o stream ao mixer!\n");
        }
        al_set_audio_stream_gain(bgm_stream, 0.7); // Volume ajustado para 0.7 (ou o que preferir)
        al_set_audio_stream_playing(bgm_stream, true); // Começa a tocar o BGM
    }

    game_state=MENU;

	//inicie a logica pro jogo
	while(!sair_programa){

	al_wait_for_event(queue, &event); // Espera por um evento
	if(event.type == ALLEGRO_EVENT_TIMER) {
            al_get_keyboard_state(&keyState); // Atualiza o estado do teclado
	}
    if(game_state==MENU){

            reset_game_state();

            al_draw_bitmap(fundo_menu, 0, 0, NULL); // desenha o fundo da tela inicial
            //al_draw_text(fonte_titulo, al_map_rgb(0,0,0), 396, 100, ALLEGRO_ALIGN_CENTER, "VIDA UNIVERSITÁRIA");
            //al_draw_text(fonte_titulo, al_map_rgb(255,255,255), 400, 100, ALLEGRO_ALIGN_CENTER, "VIDA UNIVERSITÁRIA");
            //"duplicado" pois criei uma em preto e outra em branco pra ser meio que a borda e dar contraste com a nuvem
            al_draw_text(fonte_opcoes, al_map_rgb(0,0,0), 416, 320, ALLEGRO_ALIGN_CENTER, "Pressione ENTER para jogar");
            al_draw_text(fonte_opcoes, al_map_rgb(0,0,0), 410, 415, ALLEGRO_ALIGN_CENTER, "Pressione ESC para sair");
            al_draw_text(fonte_opcoes, al_map_rgb(255,255,255), 420, 320, ALLEGRO_ALIGN_CENTER, "Pressione ENTER para jogar");
            al_draw_text(fonte_opcoes, al_map_rgb(255,255,255), 414, 415, ALLEGRO_ALIGN_CENTER, "Pressione ESC para sair");;
            al_draw_textf(fonte_titulo, al_map_rgb(0,0,0), 396, 500, ALLEGRO_ALIGN_CENTER, "RECORDE NA SESSÃO: %d",record);
            al_draw_textf(fonte_titulo, al_map_rgb(255,255,0), 400, 500, ALLEGRO_ALIGN_CENTER, "RECORDE NA SESSÃO: %d",record);
            // joga na tela



            if(al_key_down(&keyState, ALLEGRO_KEY_ESCAPE)){
                sair_programa = true;
            }
            if(al_key_down(&keyState, ALLEGRO_KEY_ENTER)){
                game_state=PLAYING;
            }

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

            if(al_key_down(&keyState, ALLEGRO_KEY_P)){
                game_state = PAUSE;
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
            atualizar_quadrado(&unicoquadrado, coordenadasobjs, disp);

            redesenhar = true;



        // Se o evento for fechar a janela
         if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE || al_key_down(&keyState, ALLEGRO_KEY_ESCAPE)) {
            sair_programa = true;

        }else if(game_state==GAME_OVER){
            if (al_key_down(&keyState, ALLEGRO_KEY_ENTER)) { // Quando ENTER é pressionado
                    reset_game_state(); // A função de reiniciar é chamada aqui!
                    redesenhar = true;  // Força um redesenho para a nova partida
            }

            draw_game_over_screen(font, font1, &unico_objeto, altura_tela);
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
            //desenha a imgaem por cima do quadrado
            if(unicoquadrado.ativo){
                 al_draw_bitmap_region(objetos1,
                     coordenadasobjs[frame_obj][0], coordenadasobjs[frame_obj][1], //coordenadas da imagem oridinal
                     coordenadasobjs[frame_obj][2], coordenadasobjs[frame_obj][3],
                     unicoquadrado.posicao.x, unicoquadrado.posicao.y, // psociao onde a imgem vai aparecer
                     0 // não precisa ser redimensionada
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
            al_map_rgb(155,155,155), //cor: amarelo
            800/2 + 3,                 //centro da tela
            43,                    //posição y
            ALLEGRO_ALIGN_CENTER,  //centralizar
            "VIDA: %d",
            vida
        );
        al_draw_textf(
            font,
            al_map_rgb(255,0,0), //cor: amarelo
            800/2,                 //centro da tela
            40,                    //posição y
            ALLEGRO_ALIGN_CENTER,  //centralizar
            "VIDA: %d",
            vida
        );
        al_draw_textf(
            font,
            al_map_rgb(150,150,150), //cor: amarelo
            800/2 + 3,                 //centro da tela
            10,                    //posição y- perto do topo
            ALLEGRO_ALIGN_CENTER,  //centralizar
            "PONTUAÇÃO: %d",
            pontuacao
        );

        al_draw_textf(
            font,
            al_map_rgb(255,255,0), //cor: amarelo
            800/2,                 //centro da tela
            10,                    //posição y- perto do topo
            ALLEGRO_ALIGN_CENTER,  //centralizar
            "PONTUAÇÃO: %d",
            pontuacao
        );

        al_draw_text(fonte_pause, al_map_rgba(0,0,0,140), 116, 30, ALLEGRO_ALIGN_CENTER, "P PARA PAUSAR");

    }else if(game_state == PAUSE){
            al_draw_bitmap(fundo_menu, 0, 0, NULL); // desenha o fundo da tela inicial;
            al_draw_text(fonte_titulo, al_map_rgb(0,0,0), 396, 265, ALLEGRO_ALIGN_CENTER, "JOGO PAUSADO");
            al_draw_text(fonte_titulo, al_map_rgb(255,255,255), 400, 265, ALLEGRO_ALIGN_CENTER, "JOGO PAUSADO");
            al_draw_text(fonte_opcoes, al_map_rgb(0,0,0), 416, 320, ALLEGRO_ALIGN_CENTER, "Pressione ENTER para RETOMAR");
            al_draw_text(fonte_opcoes, al_map_rgb(255,255,255), 420, 320, ALLEGRO_ALIGN_CENTER, "Pressione ENTER para RETOMAR");
            al_draw_text(fonte_opcoes, al_map_rgb(0,0,0), 410, 415, ALLEGRO_ALIGN_CENTER, "Pressione M para MENU");
            al_draw_text(fonte_opcoes, al_map_rgb(255,255,255), 414, 415, ALLEGRO_ALIGN_CENTER, "Pressione M para MENU");

            /*
            al_draw_text(fonte_opcoes, al_map_rgb(0,0,0), 416, 320, ALLEGRO_ALIGN_CENTER, "Pressione ENTER para jogar");
            al_draw_text(fonte_opcoes, al_map_rgb(0,0,0), 410, 415, ALLEGRO_ALIGN_CENTER, "Pressione ESC para sair");
            al_draw_text(fonte_opcoes, al_map_rgb(255,255,255), 420, 320, ALLEGRO_ALIGN_CENTER, "Pressione ENTER para jogar");
            al_draw_text(fonte_opcoes, al_map_rgb(255,255,255), 414, 415, ALLEGRO_ALIGN_CENTER, "Pressione ESC para sair");;
            al_draw_textf(fonte_titulo, al_map_rgb(0,0,0), 396, 500, ALLEGRO_ALIGN_CENTER, "RECORDE NA SESSÃO: %d",record);
            al_draw_textf(fonte_titulo, al_map_rgb(255,255,0), 400, 500, ALLEGRO_ALIGN_CENTER, "RECORDE NA SESSÃO: %d",record);
            */



            if(al_key_down(&keyState, ALLEGRO_KEY_ENTER)){
                game_state=PLAYING;
            }
            if(al_key_down(&keyState, ALLEGRO_KEY_M)){
                game_state=MENU;
            }
            if(al_key_down(&keyState, ALLEGRO_KEY_ESCAPE)){
                sair_programa = true;
            }

        }
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
                else if(checacolisaopers(&unicoquadrado, persx, persy, pers_larg, pers_alt, record)){
            if(unicoquadrado.ativo){//pro objeto sumir
            unicoquadrado.ativo = false;
            pontuacao++;

            if(pontuacao>record){
                record=pontuacao;
            }

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
        draw_game_over_screen(font, font1, &unico_objeto, altura_tela);
            if (al_key_down(&keyState, ALLEGRO_KEY_ENTER)) { // Quando ENTER é pressionado
                    reset_game_state(); // A função de reiniciar é chamada aqui!
                    redesenhar = true;  // Força um redesenho para a nova partida
                    game_state=PLAYING;
            }

            if(al_key_down(&keyState, ALLEGRO_KEY_ESCAPE)){
                sair_programa = true;
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
    al_destroy_audio_stream(bgm_stream);
    al_uninstall_audio();
    al_destroy_bitmap(fundo_menu);
    al_destroy_font(fonte_titulo);
    al_destroy_font(fonte_opcoes);
    al_destroy_font(fonte_pause);

    return 0;
}
int atualizar_quadrado(Objeto *unicoquadrado, int coordenadasobjs[][4], ALLEGRO_DISPLAY* disp){
    if(!unicoquadrado->ativo){
        unicoquadrado->id = rand()% (5 - 0 + 1) + 0;
        frame_obj = unicoquadrado->id;
        unicoquadrado->posicao.x = rand() % (al_get_display_width(disp)- coordenadasobjs[frame_obj][2]);
        unicoquadrado->posicao.y = 0;
        unicoquadrado->cor = al_map_rgba(255, 0, 255, 0);
        unicoquadrado->velo = 2;
        unicoquadrado->tamanho.x = coordenadasobjs[frame_obj][2];
        unicoquadrado->tamanho.y = 30;
        unicoquadrado->ativo = true;
}
    unicoquadrado->posicao.y += velocidade_blocos;
    if(unicoquadrado->posicao.y > al_get_display_height(disp)){
        unicoquadrado->ativo = false;
    }
    return 0;
}
void verificar_id(Objeto *unicoquadrado, obj_escolares *unico_obj){
    if(unicoquadrado->id==0){
        unico_obj->caderno++;
    }
    if(unicoquadrado->id==1){
        unico_obj->bloco_notas++;
    }
    if(unicoquadrado->id==2){
        unico_obj->livro++;
    }
    if(unicoquadrado->id==3){
        unico_obj->garrafa++;
    }
    if(unicoquadrado->id==4){
        unico_obj->notebook++;
    }
    if(unicoquadrado->id==5){
        unico_obj->diario++;
    }
}
void mostrar_lista(obj_escolares *unico_obj, ALLEGRO_FONT *font, ALLEGRO_FONT *font1, int altura_tela){
    al_draw_textf(font1, al_map_rgb(180, 255, 0), 800/2, 80, ALLEGRO_ALIGN_CENTER,
    "CARDAPIO COLETADO");
    al_draw_textf(font1, al_map_rgb(155, 155, 155), 800/2, 83, ALLEGRO_ALIGN_CENTER,
    "CARDAPIO COLETADO");
    al_draw_textf(font, al_map_rgb(255, 255, 255), 800/2, 100 +30, ALLEGRO_ALIGN_CENTER,
    "Livros: %d", unico_obj->livro);
    al_draw_textf(font, al_map_rgb(255, 255, 255), 800/2, 100 +60, ALLEGRO_ALIGN_CENTER,
    "NOtebook's: %d", unico_obj->notebook);
    al_draw_textf(font, al_map_rgb(255, 255, 255), 800/2, 100 +90, ALLEGRO_ALIGN_CENTER,
    "Garrafas: %d", unico_obj->garrafa);
    al_draw_textf(font, al_map_rgb(255, 255, 255), 800/2, 100 +120, ALLEGRO_ALIGN_CENTER,
    "Bloco de Notas: %d", unico_obj->bloco_notas);
    al_draw_textf(font, al_map_rgb(255, 255, 255), 800/2, 100 +150, ALLEGRO_ALIGN_CENTER,
    "Diários: %d", unico_obj->diario);
    al_draw_textf(font, al_map_rgb(255, 255, 255), 800/2, 100 +180, ALLEGRO_ALIGN_CENTER,
    "Caderno: %d", unico_obj->caderno);
    const int contador_total = 10;
    static int cor_delay =5;
    static int cor_atual = 0;
    static int cor_frame = 0;
    static ALLEGRO_COLOR cor = {0};
    cor_frame++;
    if(cor_frame>cor_delay){
        cor_frame=0;
        cor_atual++;
            if(cor_atual>contador_total){
               cor = al_map_rgb(rand()% 255, rand()%255, rand()%255);
            cor_atual=0;
            }
    }
    al_draw_text(font1, cor, 800/2, 100 + 300, ALLEGRO_ALIGN_CENTER,
            "TENTE NOVAMENTE - ENTER");
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
bool checacolisaopers(Objeto *um, int persx, int persy, int pers_larg, int pers_alt, int record){
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
void draw_game_over_screen(ALLEGRO_FONT *font, ALLEGRO_FONT *font1, obj_escolares *unico_objeto, int altura_tela){
     al_clear_to_color(al_map_rgb(0,0,0));
                al_draw_text(
                    font,
                    al_map_rgb(255, 0, 0),
                     800/ 2,
                    altura_tela - 100,
                    ALLEGRO_ALIGN_CENTER,
                    "GAME OVER");
    mostrar_lista(unico_objeto, font, font1, altura_tela);
} // Função para desenhar a tela de Game Over
void reset_game_state(void){
    pontuacao = 0;
    vida =3;
    unico_objeto.livro =0;
    unico_objeto.notebook=0;
    unico_objeto.garrafa=0;
    unico_objeto.bloco_notas=0;
    unico_objeto.diario=0;
    unico_objeto.caderno=0;
    unicoquadrado.ativo=false;
    aux_velocidade = 0;
} // Função para reiniciar todas as variáveis do jogo
