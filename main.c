#include <allegro5/allegro5.h>              /*funções estruturais iniciais */
#include <allegro5/allegro_font.h>          /* lida com fontes e desenhos de texto */
#include <allegro5/allegro_image.h>         /*manipulação de imagem*/
#include <allegro5/allegro_primitives.h>    /*para desenhar formas como retângulos e círculos */
#include <allegro5/allegro_ttf.h>           /* permite carregamento de fontes ttf e otf; cria textos bonitos */
#include <allegro5/allegro_native_dialog.h> /*para caixa de mensagem */
#include <stdio.h>                          /*entrada e saída*/
#include <string.h>
#include <stdbool.h>                        /*para utilização do tipo bool*/
#include <errno.h>                          /*habilita a var global de erro "errno"*/

int pontuacao = 0;//pontuação inicial; variavel global
int vida = 3; //quantidade de vida inicial

//Estrutura para um vetor 2D (posição e tamanho)
typedef struct{
    int x, y;
}vetor2d;

//Estrutura para o objeto caindo
typedef struct{
    vetor2d tamanho;
    vetor2d posicao;
    int velo;
    ALLEGRO_COLOR cor; // Alterado de int para ALLEGRO_COLOR
    bool ativo;
}Objeto;// referido dentro do codigo como "quadrado"; exemplo:int atualizar_quadrado(Objeto *q, ALLEGRO_DISPLAY* disp);

bool checacolisao(Objeto *um, Objeto *dois){
    //checa se há colisão no eixo x;
    bool colisaoX = um->posicao.x + um->tamanho.x >= dois->posicao.x &&
                    dois->posicao.x + dois->tamanho.x >= um->posicao.x;

    //checa se há colisão no eixo y:
    bool colisaoY = um->posicao.y + um->tamanho.y >= dois->posicao.y &&
                    dois->posicao.y + dois->tamanho.y >= um->posicao.y;
    //A colisão só ocorre se houver em ambos os eixos:
    return colisaoX && colisaoY;
}

//funcoes prototipo
int atualizar_quadrado(Objeto *q, ALLEGRO_DISPLAY* disp);

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

	ALLEGRO_FONT* font = al_create_builtin_font();//fonte padrão
	ALLEGRO_FONT* fonte_titulo = al_load_ttf_font("PressStart2P-Regular.ttf", 32, 0);//fonte press start 2p para titulo no menu
	ALLEGRO_FONT* fonte_opcoes = al_load_ttf_font("PressStart2P-Regular.ttf", 20, 0);//fonte press start 2p para opções no menu
    //parametros:nome do arquivo.ttf, tamanho da fonte, 0 op adicionais
    //criei duas fontes com o mesmo arquivo para poder ter textos com tamanhos diferentes


	al_register_event_source(queue, al_get_keyboard_event_source()); //registra na fila de evento as ações do user com o teclado;
	al_register_event_source(queue, al_get_display_event_source(disp));//registra na fila de evento as ações no display;
	al_register_event_source(queue, al_get_timer_event_source(timer));//registra na fila de evento as alterações no timer;


	al_set_window_title(disp, "Vida Universitária");


	//adicione as imagens, bitmap
	ALLEGRO_BITMAP *personagem = al_load_bitmap("personagem.bmp");
	ALLEGRO_BITMAP *background = al_load_bitmap("background.png");
	ALLEGRO_BITMAP *fundo_menu = al_load_bitmap("fundo_menu.png");

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
	int persy =380;
	//chão
	int altura_tela = al_get_display_height(disp);
	int altura_chao = altura_tela - persy;

	//personagem parte 2, imagens animadas
	int current_frame = 0;
	int total_frames = 3;
	int frame_delay = 5;//alteração de 300para 5
	int frame_count = 0;


	//variaveis para os objetos
	Objeto unicoquadrado;
	unicoquadrado.ativo = false;

	int coordenadasimg[3][4] = {
			{0, 0, 336, 571}, //frames 1: x,y,largura,altura. largura   336 - 0 = 336 , altura   571 -  0 = 571
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
	int estado_do_jogo;
    estado_do_jogo=0;//(0-tela de inicio, 1-jogando


	//inicie a logica pro jogo
	while(!sair_programa){
        al_wait_for_event(queue, &event); // Espera por um evento

        if(estado_do_jogo==0){
            //tela de inicio
            al_draw_bitmap(fundo_menu, 0, 0, NULL); // desenha o fundo da tela inicial
            al_draw_text(fonte_titulo, al_map_rgb(0,0,0), 396, 100, ALLEGRO_ALIGN_CENTER, "VIDA UNIVERSITÁRIA");
            al_draw_text(fonte_titulo, al_map_rgb(255,255,255), 400, 100, ALLEGRO_ALIGN_CENTER, "VIDA UNIVERSITÁRIA");
            //"duplicado" pois criei uma em preto e outra em branco pra ser meio que a borda e dar contraste com a nuvem
            al_draw_text(fonte_opcoes, al_map_rgb(0,0,0), 396, 350, ALLEGRO_ALIGN_CENTER, "Pressione ENTER para jogar");
            al_draw_text(fonte_opcoes, al_map_rgb(0,0,0), 396, 400, ALLEGRO_ALIGN_CENTER, "Pressione ESC para sair");
            al_draw_text(fonte_opcoes, al_map_rgb(255,255,255), 400, 350, ALLEGRO_ALIGN_CENTER, "Pressione ENTER para jogar");
            al_draw_text(fonte_opcoes, al_map_rgb(255,255,255), 400, 400, ALLEGRO_ALIGN_CENTER, "Pressione ESC para sair");

            al_flip_display(); // joga na tela


            al_get_keyboard_state(&keyState);

            if(al_key_down(&keyState, ALLEGRO_KEY_ESCAPE)){
                sair_programa = true;
            }
            if(al_key_down(&keyState, ALLEGRO_KEY_ENTER)){
                estado_do_jogo=1;
            }

        }else{
        if(estado_do_jogo==1){
           //al_wait_for_event(queue, &event); // Espera por um evento

        if(event.type == ALLEGRO_EVENT_TIMER) {
            al_get_keyboard_state(&keyState); // Atualiza o estado do teclado

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
            if(al_key_down(&keyState, ALLEGRO_KEY_M)){
                estado_do_jogo=0;
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
            /*if(persx > al_get_display_width(disp) - coordenadasimg[current_frame][2]){
                persx = al_get_display_width(disp) - coordenadasimg[current_frame][2];
            }*/

            //o bug na hora de virar a boneca pra esquerda me faz pensar q talvez o codigota contando a boneca grande
            //sendo que o kauha redimensionou pra ela ficar menorzinha

            float largura_redimensionada;//cria uma variavel para compensar, "na largura do sprite" nao sei se e a forma correta de se falar, o fato de que ela foi diminuida
            largura_redimensionada = coordenadasimg[current_frame][2]*0.2;//compensa a largura, tambem com 0.2 pra ficar proporcional perfeitinho
/*
            if(persx>al_get_display_width(disp)- largura_redimensionada){
                persx =al_get_display_width(disp)- largura_redimensionada;
            }//meio que muda so que substitui os coordenadasimg... por largura redimensionada para compensar
*/

            if(vira_esquerda){
                    if(persx<-largura_redimensionada){
                        persx = -largura_redimensionada;
                    }
            }else{
                if(persx>al_get_display_width(disp)-largura_redimensionada){
                persx = al_get_display_width(disp)-largura_redimensionada;
                }
            }//agora ela fica travainha na esquerda, mas ultrapassa a tela mesmo assim, mas pelo menos ta um pouco melhor

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
                     unicoquadrado.posicao.x, unicoquadrado.posicao.y,
                     unicoquadrado.posicao.x + unicoquadrado.tamanho.x, unicoquadrado.posicao.y + unicoquadrado.tamanho.y,
                     unicoquadrado.cor
                 );
            }
/*
            // Desenha a região correta do personagem (spritesheet)
            al_draw_scaled_bitmap(
                personagem,
                coordenadasimg[current_frame][0] , coordenadasimg[current_frame][1],
                coordenadasimg[current_frame][2] , coordenadasimg[current_frame][3],
                persx, persy,
                coordenadasimg[current_frame][2]*0.2, coordenadasimg[current_frame][3] *0.2,
                0);
*///comentando a parte que faz o desenho para fazer um teste da personagem virando de lado

            if(vira_esquerda==true){
    al_draw_scaled_bitmap(
        personagem,
        coordenadasimg[current_frame][0], coordenadasimg[current_frame][1],  // igual
        coordenadasimg[current_frame][2], coordenadasimg[current_frame][3],  // igual
        persx + coordenadasimg[current_frame][2]*0.2// mas x tem que ser compensada para que a personagem fiqie posiionada corretamente e se nao tiver esse 0.2 ela teleporta qando vira
        , persy,  // posição y pode continuar a mesma
        -coordenadasimg[current_frame][2]*0.2, coordenadasimg[current_frame][3]*0.2,0);  // negativa a largura pra espelahr, deixa a mesma altura
//ago
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
            //agora ela vira para a esquerda mas quando anda da umas mini travadinhas nao sei o que fazer
            //outra coisa, antes de adicionar o *0.2, estava bugado e talvez eu tenha tido uma ideia de pq ela nao vai ate o final da direita
            //apesar diss, ela continua indo ate o final da esquerda como antes


            al_draw_text(fonte_opcoes, al_map_rgb(0,0,0), 16,580 , ALLEGRO_ALIGN_LEFT, "Pressione M para voltar ao menu");
            al_draw_text(fonte_opcoes, al_map_rgb(255,255,255), 20,580 , ALLEGRO_ALIGN_LEFT, "Pressione M para voltar ao menu");

            // Joga tudo que foi desenhado na tela
            al_flip_display();
            //substitui a tela anteiro
            al_clear_to_color(al_map_rgb(0,0,0));
        }

        if(checacolisao(&unicoquadrado, &altura_chao)){
            vida--;
        }
        if(checacolisao(&unicoquadrado, &personagem)){
            pontuacao++;
        }

        if(vida == 0){
            al_clear_to_color(al_map_rgb(0, 0, 0));
            al_draw_text(
                font,
                al_map_rgb(255, 0, 0),
                 800/ 2,
                altura_tela / 2 - 80,
                ALLEGRO_ALIGN_CENTER,
                "GAME OVER"
            );
        }
            }
        }


    }


    // Limpeza de recursos
    al_destroy_bitmap(personagem);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_bitmap(fundo_menu);
    al_destroy_font(fonte_titulo);
    al_destroy_font(fonte_opcoes);
    return 0;
}


int atualizar_quadrado(Objeto *q, ALLEGRO_DISPLAY* disp){
    if(!q->ativo){
        q->posicao.x = rand() % (al_get_display_width(disp) - 30);
        q->posicao.y = 0;
        q->cor = al_map_rgb(rand()%256, rand()%256, rand()%256);
        q->velo = 2 + (rand()%3);
        q->tamanho.x = rand()%16+5;
        q->tamanho.y = rand()%16+5;
        q->ativo = true;
    }

    q->posicao.y += q->velo;

    if(q->posicao.y > al_get_display_height(disp)){
        q->ativo = false;
        return 0;
    }

    return 1;
}
