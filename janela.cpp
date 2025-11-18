#include <GL/glu.h> // Inclui a biblioteca GLUT (pode ser <GLUT/glut.h> no macOS)
#include <math.h>
#include <cstdio>
#include <vector>     // Para a "lista" de obstáculos
#include <stdlib.h>   // Para rand() (posições aleatórias)
#include <time.h>
#include <iostream>
#include <gl/GL.h>
#include <GL/freeglut_std.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
    
    
struct modelFrame{
    std::vector <float> vertices;
    std::vector <float> normais;
    std::vector<float> cord_text;
};

std::vector<modelFrame> lirili;
std::vector<modelFrame> tralala;
modelFrame capuccina; // essa pode ser apenas um modelo
modelFrame bomb; // aqui tambem!
// variaveis para animacao globais
std::vector<modelFrame> runAnimation; // vetor que guarda todos os frames
int currentRunFrame = 0;    // O índice do frame atual
GLuint tex_jogador = 0;      
GLuint li_tex = 0;
GLuint tra_tex = 0;
GLuint cap_tex = 0;
GLuint bomb_tex = 0;    // indc da textura atual"
float animationTimer = 0.0f;          // Timer para controlar a velocidade
const float TIME_PER_FRAME = 0.1f;
float ang = 20;

// Variáveis globais para a posição do jogador (no futuro)
float playerX = 0.0f;
float playerY = 0.5f; // O cubo tem 1.0 de altura, então seu centro fica 0.5 acima do chão
float playerZ = 0.0f;

// variaveis para o movimento:
enum TipoObstaculo{TRA, CAP, BOMB_front, BOMB_back, TUNG, JUMP};
float WorldZPos = 0.0f; // quanto o mundo ja andou
float WorldSpeed = 10.0f;
float WorldZPosAnt = 0.0f;
int last_time = 0; // tempo do ultimo frame

const float tile = 20.0f; // comprimento do chao

// variaveis de controle

bool keyA = false;
bool keyD = false;
const float p_speed = 5.0f;



struct obstaculo{
    float x, y, z;
    TipoObstaculo tipo;
    bool movimento_cap;
    int currentFrame;
    float animationTime;
};

std::vector<obstaculo> obstaculos;




// (Lembre-se de incluir <string> e <iostream>)

GLuint loadTexture(const std::string& filepath, bool inverter_textura) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(inverter_textura);
    // Carrega a imagem do disco
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    if (data == NULL) {
        std::cerr << "Erro ao carregar a textura: " << filepath << std::endl;
        return 0; // Retorna 0 (ID inválido)
    }

    // Determina o formato da imagem (RGB ou RGBA com transparência)
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    GLuint textureID;
    glGenTextures(1, &textureID);       // 1. Gera um ID para a textura
    glBindTexture(GL_TEXTURE_2D, textureID); // 2. "Amarra" o ID ao alvo GL_TEXTURE_2D

    // 3. Define os parâmetros da textura
    // O que fazer se a textura for menor/maior que o objeto
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Como misturar pixels (interpolação linear fica mais suave)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 4. Envia os dados da imagem (data) para a GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // 5. Libera a memória da imagem da CPU (ela já está na GPU)
    stbi_image_free(data);

    // Desamarra a textura
    glBindTexture(GL_TEXTURE_2D, 0);

    std::cout << "Textura carregada: " << filepath << std::endl;
    return textureID; // Retorna o ID da textura
}



modelFrame loadModelFrame(const std::string& filepath) {
    modelFrame modelFrame; // Nosso frame vazio

    // 1. Estruturas da TinyObjLoader
    tinyobj::attrib_t attrib; // Guarda os vértices, normais, texcoords
    std::vector<tinyobj::shape_t> shapes; // Guarda as "partes" do modelo
    std::vector<tinyobj::material_t> materials; // (Não vamos usar)
    std::string warn, err;

    // 2. Chama a função de carregamento
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
        // Se deu erro, imprime e retorna um frame vazio
        std::cerr << "Erro ao carregar o .obj: " << filepath << std::endl;
        std::cerr << warn << err << std::endl;
        return modelFrame; // Retorna vazio
    }

    // 3. A "GINCANA" - Traduzindo os dados
    // Percorre cada "shape" (parte) do modelo
    for (const auto& shape : shapes) {
        // Percorre cada "índice" (vértice) nessa parte
        for (const auto& index : shape.mesh.indices) {
            
            // --- Pega o Vértice (Posição) ---
            // 'index.vertex_index' é o índice na lista de vértices do .obj
            // Multiplicamos por 3 (pois é X, Y, Z) e pegamos os 3
            modelFrame.vertices.push_back(attrib.vertices[3 * index.vertex_index + 0]); // X
            modelFrame.vertices.push_back(attrib.vertices[3 * index.vertex_index + 1]); // Y
            modelFrame.vertices.push_back(attrib.vertices[3 * index.vertex_index + 2]); // Z

            // --- Pega a Normal ---
            // 'index.normal_index' é o índice na lista de normais
            if (index.normal_index >= 0) { // Checa se a normal existe
                modelFrame.normais.push_back(attrib.normals[3 * index.normal_index + 0]); // X
                modelFrame.normais.push_back(attrib.normals[3 * index.normal_index + 1]); // Y
                modelFrame.normais.push_back(attrib.normals[3 * index.normal_index + 2]); // Z
            }

            if (index.texcoord_index>=0){
                modelFrame.cord_text.push_back(attrib.texcoords[2*index.texcoord_index+0]);
                modelFrame.cord_text.push_back(attrib.texcoords[2*index.texcoord_index+1]);
            }


        }
    }

    std::cout << "Modelo carregado: " << filepath << " (" << modelFrame.vertices.size() / 3 << " vertices)" << std::endl;
    return modelFrame; // Retorna o frame preenchido!
}




// --- Função de Inicialização ---
// Configura estados iniciais do OpenGL
void init() {
    // Define a cor de fundo (céu azul claro)
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);

    
    
    // Habilita o teste de profundidade (Z-buffer)
    // Isso faz com que objetos mais próximos cubram os mais distantes
    glEnable(GL_DEPTH_TEST);


    // criacao de objetos vindos de fora:

    srand(time(NULL));
    obstaculos.push_back({0.0f, 0.5f,-60.0f, CAP, false, 0, 0.0f});
    obstaculos.push_back({5.0f, 0.5f,-80.0f, TRA, false, 0, 0.0f});
    obstaculos.push_back({-3.0f, 0.5f,-30.0f, CAP, false, 0, 0.0f});
    obstaculos.push_back({-3.0f, 0.5f,-40.0f, TRA, false, 0, 0.0f});
}
    

    // coloco na lista de obstaculos os obstaculos criados



void desenharModel(const modelFrame& quadro){
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, quadro.vertices.data());
    if (!quadro.normais.empty()) {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, quadro.normais.data());
    }

    if (!quadro.cord_text.empty()){
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT,0,quadro.cord_text.data());
    }

    glDrawArrays(GL_TRIANGLES, 0, quadro.vertices.size()/3);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}



void update(void){

    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - last_time) / 1000.0f;
    last_time = currentTime;

    // definicao do aumento da velocidade do mundo!
    if (WorldZPos > WorldZPosAnt+100.0f && WorldZPosAnt!= 1.0f){
        WorldSpeed+=5.0f;
        WorldZPosAnt = WorldZPos;
    }
    else if(WorldZPos > 600.0f)
        WorldZPosAnt=1.0f;
    WorldZPos+=WorldSpeed*deltaTime;
    

    if (keyA){
        playerX-= p_speed*deltaTime; // coloca a velocidade do jogador em funcao do tempo ao pressionar a tecla   
    }
    if (keyD){
        playerX+= p_speed*deltaTime;
    }

    // para limitar o movimento do personagem, posso:
    const float larg_pista = 8.0f;
    if (playerX>larg_pista-1){
        playerX= larg_pista-1;
    }
    if (playerX < -larg_pista+1){
        playerX=-larg_pista+1;
    }

    for (int i = 0; i < obstaculos.size();i++){

        // colocar o objeto pra se mover na mesma velocidade do mundo
        obstaculos[i].z += WorldSpeed*deltaTime; // o Z do objeto vai se mover de acordo com a velocidade do mundo e o passar do tempo
        if (obstaculos[i].tipo == CAP && obstaculos[i].movimento_cap == false){
            obstaculos[i].x += 3.0f*deltaTime;
        }
        else if (obstaculos[i].tipo == CAP && obstaculos[i].movimento_cap == true){
            obstaculos[i].x -= 3.0f*deltaTime;
            
        }

        if ( obstaculos[i].x > larg_pista){
                obstaculos[i].movimento_cap = true;
            }
             if (obstaculos[i].x < -larg_pista){
                obstaculos[i].movimento_cap = false;
            }
        
        if (obstaculos[i].tipo == BOMB_front)
                obstaculos[i].x+=3.0f*deltaTime;
        else if (obstaculos[i].tipo == BOMB_back)
                obstaculos[i].x-=3.0f*deltaTime;
        

        // update de obstaculos animados:
            if (obstaculos[i].tipo == TRA || obstaculos[i].tipo == TUNG){
                    obstaculos[i].animationTime+=deltaTime;
                    if (obstaculos[i].animationTime>=TIME_PER_FRAME){
                        obstaculos[i].animationTime = 0;
                        obstaculos[i].currentFrame++;


                        if (obstaculos[i].currentFrame>=lirili.size()){
                            obstaculos[i].currentFrame = 0;
                        }

                    }

            }  
            
        


        if (obstaculos[i].z>15.0f){

            obstaculos[i].z = -100.0f;
            // rand() % 16 gera numeros aleatorios entre 0 e 15
            // rand()%16 - y gera numeros aleatorios entre 0-y e 16-y-1
            obstaculos[i].x = (rand() % 16) - 7.0f; // gera numeros aleatorios entre 0-8 (-8) e 16-8-1 (7 nesse caso)
           int tipoSorteado;
            if (WorldZPos<500.0f){
            tipoSorteado = rand() % 3;
            }else{
            tipoSorteado = rand() % 6; 
            }
    // 3. Configura TUDO baseada no tipo (incluindo a altura Y)
    switch(tipoSorteado) {
        case 0: 
            obstaculos[i].tipo = TRA;
            obstaculos[i].y = 0.5f;  // <--- IMPORTANTE: Define altura do chão
            obstaculos[i].currentFrame = 0; // reseta o frame e o tempo
            obstaculos[i].animationTime = 0;
            break;
            
        case 1: 
            obstaculos[i].tipo = TUNG;
            obstaculos[i].y = 0.5f; // <--- Define altura do chão
            obstaculos[i].currentFrame = 0;
            obstaculos[i].animationTime = 0;
            break;
            
        case 2: 
            obstaculos[i].tipo = CAP;
            obstaculos[i].y = 0.5f; // <--- Define altura do chão
            obstaculos[i].movimento_cap = (rand() % 2 == 0);
            break;

        case 3: 
            obstaculos[i].tipo = BOMB_front;
            obstaculos[i].y = 3.5f; // <--- AQUI! Define a altura do VOO (ex: 3.5)
            break;

        case 4: 
            obstaculos[i].tipo = BOMB_back;
            obstaculos[i].y = 3.5f; // <--- AQUI TAMBÉM
            break;
        case 6:
            obstaculos[i].tipo = JUMP;
            obstaculos[i].y = 0.5f; // aqui vai forcar o o jogador a pular, vou colocar um objeto q pegue todo o chao
            break;
    }
}
   

}

animationTimer+=deltaTime;
 if (animationTimer>=TIME_PER_FRAME){
    animationTimer = 0.0f;
    currentRunFrame++;

    if (currentRunFrame>=runAnimation.size()){
        currentRunFrame = 0; // resetar a animacao
    }
}
glutPostRedisplay();
}


void keyboardDown(unsigned char key, int x, int y){
    switch (key){
        case 'A': keyA = true; break;
        case 'a': keyA = true; break;
        case 'D': keyD = true; break;
        case 'd': keyD = true; break;
    }
    glutPostRedisplay();
}
void keyboardUp (unsigned char key, int x, int y){
    switch (key){
        case 'A': keyA = false; break;
        case 'a': keyA = false; break;
        case 'D': keyD = false; break;
        case 'd': keyD = false; break;
    }
}
// --- Função de Desenho (Display) ---
// Chamada toda vez que a tela precisa ser redesenhada
void display() {
    // 1. Limpa os buffers de cor e profundidade
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. Reseta a matriz ModelView (transformações de modelo e câmera)
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity(); // Começa com a matriz identidade (sem transformações)

    // 3. Configura a Câmera (A parte mais importante!)
    // Usamos gluLookAt para definir a posição da câmera e para onde ela olha.
    // Posição da Câmera (eye): (0, 5, 10) -> X=0 (centralizada), Y=5 (acima), Z=10 (atrás)
    // Para Onde Olha (center): (0, 0, 0) -> Olhando para a origem, onde o jogador está
    // Vetor "Up" (para cima): (0, 1, 0) -> O eixo Y é o "para cima" do mundo
    gluLookAt(
        0.0, 5.0, 10.0,  // Posição da câmera (eyeX, eyeY, eyeZ)
        0.0, 0.5, 0.0,   // Ponto para onde a câmera olha (centerX, centerY, centerZ)
        0.0, 1.0, 0.0    // Vetor "para cima" (upX, upY, upZ)
    );

    glPushMatrix(); //inicio do movimento do mundo

    float offZset = fmod(WorldZPos,tile);
    int numTilesPassados = (int)floor(WorldZPos/tile); // posicao dividida pelo tamanho dos chaos
    
    glTranslatef(0.0f,0.0f,offZset);

    // 4. Desenha o Chão
   
    glColor3f(0.0f, 0.6f, 0.2f); // Cor verde
    for (float z = 20.0f; z>-400.0f; z-=tile){
           int tile_relativo = (int)round(z/tile);
           int tile_absoluto = numTilesPassados+tile_relativo;
           if (tile_absoluto%2==0){
            glColor3f(0.0f,0.6f,0.2f);
           }
           else{
            glColor3f(0.1f,0.7f,0.3f);

           }

    glBegin(GL_QUADS);
        glVertex3f(-10.0f, 0.0f, z);
        glVertex3f(10.0f, 0.0f, z);
        glVertex3f(10.0f, 0.0f, z-tile);
        glVertex3f(-10.0f, 0.0f, z-tile);
        glEnd();
        }
    glPopMatrix();
        
        
        for(auto i : obstaculos){
        glPushMatrix();
        glTranslatef(i.x,i.y,i.z);
        if (i.tipo == JUMP){
            glPushMatrix();
            glColor3f(1.0f,0.5f,0.9f);
            glutSolidCube(1);
            glPopMatrix();
        }
        if (i.tipo == TRA || i.tipo == TUNG){
            glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glColor3f(1.0f,1.0f,1.0f);
                
           glScalef(200.0f,200.0f,200.0f);
            glBindTexture(GL_TEXTURE_2D, li_tex);
            desenharModel(lirili[i.currentFrame]);
            glDisable(GL_TEXTURE_2D);
            glPopMatrix();
        }
       
        if(i.tipo == CAP){
            glPushMatrix();
            glEnable(GL_TEXTURE_2D);
            glColor3f(1.0f,1.0f,1.0f);
            glScalef(100.0f,100.0f,100.0f);
            ang+=0.5;
            if (ang>359) ang-=359;
            glRotatef(ang,0,1,0);
            glBindTexture(GL_TEXTURE_2D,cap_tex);
            desenharModel(capuccina);
            glDisable(GL_TEXTURE_2D);
           glPopMatrix();
       }
        glColor3f(0.0f,0.0f,1.0f);
       if(i.tipo == BOMB_front || i.tipo == BOMB_back){
           // glTranslatef(i.x, i.y, i.z);
            glutSolidTeapot(1);
       }
        
        glPopMatrix();    
        }
        
    // 5. Desenha o Jogador (um cubo)
    glPushMatrix();
         // Salva a matriz atual (para não afetar outros objetos)
        // Define a cor do jogador (branco)
        glColor3f(1.0f, 1.0f, 1.0f);
       
        // Move o cubo para a posição inicial do jogador
        glTranslatef(playerX, playerY, playerZ);
       
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex_jogador);
        // Desenha um cubo sólido com tamanho 1.0
        if(!runAnimation.empty()){
            glScalef(100.0f, 100.0f, 100.0f); 
            glRotatef(-180,0,1,0);

            desenharModel(runAnimation[currentRunFrame]);
        }
        else{
        glutSolidCube(1.0);
        }
        glDisable(GL_TEXTURE_2D);
    glPopMatrix(); // Restaura a matriz anterior


        // desenhar o numero ja caminhado
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            gluOrtho2D(0,800,0,600);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            glDisable(GL_DEPTH_TEST);

            char texto[50];
            sprintf(texto, "%.0f", WorldZPos);

            glColor3f(1.0f,1.0f,1.0f);
            glRasterPos2f(10,580);

            for (const char* c = texto; *c != '\0'; c++){
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
            }

            glEnable(GL_DEPTH_TEST);
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();

    // 6. Troca os buffers (Double Buffering)
    // Mostra o que acabamos de desenhar
    glutSwapBuffers();
}

// --- Função Reshape ---
// Chamada quando a janela é redimensionada
void reshape(int w, int h) {
    // Evita divisão por zero se a janela for muito pequena
    if (h == 0) h = 1;
    float ratio = 1.0f * w / h;

    // 1. Define a área de visualização (Viewport)
    glViewport(0, 0, w, h);

    // 2. Configura a Matriz de Projeção (Perspectiva)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Define a perspectiva:
    // 45.0 = Ângulo de visão (FOV - Field of View)
    // ratio = Aspecto da tela (largura/altura)
    // 1.0 = Distância do "near clipping plane" (mínima distância visível)
    // 1000.0 = Distância do "far clipping plane" (máxima distância visível)
    gluPerspective(45.0f, ratio, 1.0f, 1000.0f);

    // Volta para a matriz ModelView para a função display()
    glMatrixMode(GL_MODELVIEW);
}


// --- Função Principal (Main) ---
int main(int argc, char** argv) {
    // Inicializa o GLUT
    glutInit(&argc, argv);

    runAnimation.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\tungRunner\\Tung_run01obj.obj"));
    runAnimation.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\tungRunner\\Tung_run02obj.obj"));
    runAnimation.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\tungRunner\\Tung_run03obj.obj"));
    runAnimation.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\tungRunner\\Tung_run04obj.obj"));
    runAnimation.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\tungRunner\\Tung_run05obj.obj"));
    runAnimation.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\tungRunner\\Tung_run06obj.obj"));
    runAnimation.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\tungRunner\\Tung_run07obj.obj"));
    runAnimation.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\tungRunner\\Tung_run08obj.obj"));
    runAnimation.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\tungRunner\\Tung_run09obj.obj"));
    runAnimation.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\tungRunner\\Tung_run10obj.obj"));
    runAnimation.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\tungRunner\\Tung_run11obj.obj"));
    runAnimation.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\tungRunner\\Tung_run12obj.obj"));
    // jogador acima
    

    lirili.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\LiriliRunner\\Lirili01.obj"));
    lirili.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\LiriliRunner\\Lirili02.obj"));
    lirili.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\LiriliRunner\\Lirili03.obj"));
    lirili.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\LiriliRunner\\Lirili04.obj"));
    lirili.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\LiriliRunner\\Lirili05.obj"));
    lirili.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\LiriliRunner\\Lirili06.obj"));
    lirili.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\LiriliRunner\\Lirili07.obj"));
    lirili.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\LiriliRunner\\Lirili08.obj"));
    lirili.push_back(loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\LiriliRunner\\Lirili09.obj"));

    capuccina = loadModelFrame("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\cap_dancer.obj");



    // Define o modo de display:
    // GLUT_DOUBLE = Double buffering (evita flickering)
    // GLUT_RGB = Modo de cor RGB
    // GLUT_DEPTH = Habilita o buffer de profundidade
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Define o tamanho e posição inicial da janela
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    
    // Cria a janela com um título
    glutCreateWindow("Passo 2: movement world");

    // Registra as funções de "callback"
    glutDisplayFunc(display);   // Função de desenho
    glutReshapeFunc(reshape);   // Função de redimensionamento
    glutIdleFunc(update);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    last_time = glutGet(GLUT_ELAPSED_TIME);
     
    // Chama a nossa função de inicialização
    init();
        tex_jogador = loadTexture("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\gltf_embedded_0.jpeg", false);
        li_tex = loadTexture("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\texture_0.png", true);
        cap_tex = loadTexture("C:\\Users\\arthu\\OneDrive\\Desktop\\CGI\\Jogo\\texture_diffuse.png", true);
    // Inicia o loop principal do GLUT
    // O programa fica aqui, esperando por eventos (mouse, teclado, etc.)
    glutMainLoop();
    
    return 0;
}