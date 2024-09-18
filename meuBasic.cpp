#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <unordered_map>

using namespace std;

/*
Como vou tratar a situação do operador GOTO?
-> Como o arquivo .txt está sendo carregado para um buffer, podemos depois ir lendo esse buffer 
até encontrar uma label, quando encontrado, iremos inserir em uma tabela qual a label e qual o valor do buffer 
onde está ela. Assim, quando surgir um 'goto _N°_', ele vai verificar se esse número está na tabela.
PENSAR EM COMO MELHORAR ESSA LÓGICA NO SENTIDO DE EFICIÊNCIA!!!
Vou utilizar o <map> kk
Identificação da label: Quando encontrar um token goto, extrair o número da label
Busca na tabela: Procurar o número da label na tabela de labels
Verificação: Se a label for encontrada, ajustar o position para a posição armazenada na tabela
Tratamento de erros: Se a label não for encontrada, gerar um erro de "label não encontrada" 
*/


enum Tipo {COMANDO, IDENTIFICADOR, NUMERO, OPERADOR, DESCONHECIDO, LABEL, REM, COMENTARIO, STRING, DOISPONTOS};

struct Token {
    Tipo tipo;
    string valor;
};

struct Simbolo {
    string nome;
    Tipo tipo;
    string valor;
};

vector<Tipo> meus_tokens;

unordered_map<string, Simbolo> tabela_simbolos; // Tabela de símbolos
unordered_map<string, size_t> tabela_labels; // Tabela de labels para o goto


/*Funções auxiliares booleanas com um regex para avaliarmos se temos algum tipo*/

bool Palavra_Chave(string token) {
    regex palavra_chave("\\b(LET|PRINT|IF|THEN|INPUT|GOTO|HALT|:)\\b");
    return regex_match(token, palavra_chave);
}

bool Comentario(string token){
    regex comentario("^REM.*|//.*");
    return regex_match(token, comentario);
}

bool Identificador(string token) {
    regex identificador("[a-zA-Z][a-zA-Z0-9_]*");
    return regex_match(token, identificador);
}

bool Label(string token, bool inicio_linha) {
    
    return inicio_linha && regex_match(token, regex("^\\d+$"));
}

bool Numero(string token) {
    regex numero("[-+]?[0-9]*\\.?[0-9]+");
    return regex_match(token, numero);
}

bool Operador(string token) {
    regex operador("[+\\-*/=<>]");
    return regex_match(token, operador);
}

bool StringLiteral(string token) {
    return regex_match(token, regex("\"(\\.|[^\"])*\"")); // Permite caracteres 
}

Tipo classificaTipo(string token, bool inicio_linha) {
    if (Label(token, inicio_linha)){
        return LABEL;
    } else if (Palavra_Chave(token)) {
        return COMANDO;
    } else if (Identificador(token)){
        return IDENTIFICADOR;
    } else if (Numero(token)) {
        return NUMERO;
    } else if (Operador(token)){
        return OPERADOR;
    } else if (Comentario(token)){
        return REM;
    } else if (StringLiteral(token)){
        return STRING;
    } else {
        return DESCONHECIDO;
    }
}
string tipoParaString(Tipo tipo) {
    switch (tipo)
    {
    case COMANDO: return "COMANDO";
    case IDENTIFICADOR: return "IDENTIFICADOR";
    case NUMERO: return "NUMERO";
    case OPERADOR: return "OPERADOR";
    case DESCONHECIDO: return "DESCONHECIDO";
    case LABEL: return "LABEL";
    case REM: return "REM";
    case COMENTARIO: return "COMENTARIO";
    case STRING: return "STRING";
    default: return "DESCONHECIDO";
    }
}

/*Vamos criar uma função para dividir todo nosso buffer em tokens de acordo com os espaços em branco
Para que podemos classificar cada token*/
vector<string> dividirEmTokens(const string& buffer){
    vector<string> tokens;
    string atual;

    for (char c : buffer) {
        if (isspace(c)) {
            if (!atual.empty()) {
                tokens.push_back(atual);
                atual.clear();
            }
            if (c == '\n') {
                tokens.push_back("\n");
            }
        } else {
            atual += c;
        }
    }
    if (!atual.empty()){
        tokens.push_back(atual);
    }

    return tokens;
}

void preencherTabelaDeLabels(const vector<Token>& tokens_classificados) {
    size_t linha = 0;
    //bool nova_linha = true;
    for (size_t i = 1; i < tokens_classificados.size(); ++i) {
        if (tokens_classificados[i].tipo == LABEL) {
            tabela_labels[tokens_classificados[i].valor] = i;
            //cout << "Label: " << tokens_classificados[i].valor << " na linha: " << i << endl;
        }
        
    }
}

/*Função com switch case com o tipo criados para preencher o vetor de tokens:*/
void preencherVetorTokens(const vector<string>& tokens, vector<Token>& tokens_classificados){
    bool inicio_linha = true;
 
    for (const string& token : tokens) {
        if (token == "\n") {
            inicio_linha = true;
            continue;
        }
        Token t;
        t.valor = token;
        t.tipo = classificaTipo(token, inicio_linha);
        tokens_classificados.push_back(t);

        /*DEPURAÇÂO LEMBRAR DE RETIRAR*/
        //cout << "Token: " << t.valor << ", Tipo: " << tipoParaString(t.tipo) << endl;

        if (token == "\n") {
            inicio_linha = true;
        } else {
            inicio_linha = false;
        }
        /*Se encontrarmos uma nova linha, o próximo token será o início da linha*/
        //inicio_linha = false;
    }

}

/*Função para buscar valor de uma variável ou número*/
double buscarValor(const string& token) {
    if (Numero(token)) {
        return stod(token); // Converte o token em um número
    } else if (tabela_simbolos.find(token) != tabela_simbolos.end()) {
        /*Busca o valor da variável*/
        return stod(tabela_simbolos[token].valor); // Converte o valor da variável em número
    } else {
        //cout << "Erro: Variável '" << token << "' não definida.\n";
        return 0;
    }
}

/*VERIFICAR COMANDOS e PROCESSAR ELES*/

bool ComandoPrint(const string& token){
    return token == "PRINT";
}

bool ComandoInput(const string& token) {
    return token == "INPUT";
}

bool ComandoAtribuicao(const vector<Token>& tokens, size_t i){
    return tokens[i].tipo == IDENTIFICADOR && i + 1 < tokens.size() && tokens[i + 1].valor == "=";
}

bool ComandoIF(const string& token) {
    return token == "IF";
}

bool DoisPontos(const string& token) {
    return token == ":";
}

bool TratarGoto(const vector<Token>& tokens_classificados, size_t& i) {
    string label_destino = tokens_classificados[i + 1].valor;
    if (tabela_labels.find(label_destino) != tabela_labels.end()) {
        /*Obter o índice da label na tabela*/
        //size_t nova_linha = tabela_labels[label_destino];
        i = tabela_labels[label_destino];
        //cout << "Saltando para a linha " << i + 1 << " (label: " << label_destino << ")" << endl;
        //i = nova_linha; // Atualizar o índice para a nova linha
        return true;
    } else {
        //cout << "Label " << label_destino << " não encontrada!" << endl;
        return false;
    }
    /*
    if (i + 1 < tokens_classificados.size() && tokens_classificados[i + 1].tipo == LABEL) {
        string label_destino = tokens_classificados[i + 1].valor;
        if (tabela_labels.find(label_destino) != tabela_labels.end()) {
            i = tabela_labels[label_destino] - 1 ;
            //DEPURAÇÂOOOOOOOOOO
            cout << "GOTO encontrado, pulando para a linha: " << i << endl;  
            return true;
        } else {
            cout << "teste Essa label não ta aqui nn: " << label_destino;
            return false;
        }
    }else {
        return false;
    }
    */
    //return false;
}

/*A análise das operações aritméticas vai ser feita com lógica de pilhas*/
double AnaliseEXPAritmetica(const vector<Token>& tokens, size_t inicio, size_t final) {
    stack<double> valores;
    stack<char> operadores;

    
    for (size_t i = inicio; i <= final; i++) {
        if (tokens[i].tipo == NUMERO) {
            valores.push(stod(tokens[i].valor));
        } else if (tokens[i].tipo == IDENTIFICADOR) {
            valores.push(buscarValor(tokens[i].valor));
        } else if (tokens[i].tipo == OPERADOR) {
            char operador = tokens[i].valor[0];
            while (!operadores.empty() && 
                   ((operador == '+' || operador == '-') || 
                    (operador == '*' || operador == '/'))) {
                char op_top = operadores.top();
                if ((op_top == '*' || op_top == '/') || 
                    ((op_top == '+' || op_top == '-') && 
                    (operador == '+' || operador == '-'))) {
                    operadores.pop();
                    double x = valores.top(); valores.pop();
                    double y = valores.top(); valores.pop();

                    if (op_top == '+') valores.push(y + x);
                    else if (op_top == '-') valores.push(y - x);
                    else if (op_top == '*') valores.push(y * x);
                    else if (op_top == '/') valores.push(y / x);
                } else {
                    break;
                }
            }
            operadores.push(operador);
        }
    }

    while (!operadores.empty())
    {
        double x = valores.top(); valores.pop();
        double y = valores.top(); valores.pop();
        char operador = operadores.top(); operadores.pop();

        if (operador == '+') {
            valores.push( x + y );
        } else if (operador == '-') {
            valores.push( x - y );
        } else if (operador == '*') {
            valores.push( x * y );
        } else if (operador == '/') {
            valores.push( x / y );
        }
    }

    return valores.top();    

}

void AnalisaIF(const vector<Token>& tokens_classificados, size_t& i) {
    /*Certifique-se de que o índice 'i' esteja no comando IF*/
    if (tokens_classificados[i].tipo != COMANDO || !ComandoIF(tokens_classificados[i].valor)) {
        cout << "Erro" << endl;
        return;
    }

    /*Identificar os operandos e o operador da condição IF*/
    string operando1 = tokens_classificados[i + 1].valor;
    string operador = tokens_classificados[i + 2].valor;
    string operando2 = tokens_classificados[i + 3].valor;

    /*Avaliar a expressão condicional*/
    double valor1 = buscarValor(operando1);
    double valor2 = buscarValor(operando2);
    bool condicao = false;

    /*Avaliar a condição com base no operador*/
    if (operador == "=") {
        condicao = (valor1 == valor2);
    } else if (operador == "<") {
        condicao = (valor1 < valor2);
    } else if (operador == ">") {
        condicao = (valor1 > valor2);
    } else if (operador == "<=") {
        condicao = (valor1 <= valor2);
    } else if (operador == ">=") {
        condicao = (valor1 >= valor2);
    } else if (operador == "!=") {  
        condicao = (valor1 != valor2);
    } else {
        cout << "Erro" << endl;
        return;
    }

    /*Exibe a comparação e resultado para depuração*/
    //cout << "Comparando: " << valor1 << " " << operador << " " << valor2 << endl;
    //cout << "Resultado da condicao: " << (condicao ? "Verdadeiro" : "Falso") << endl;

    if (condicao) {
        /*Se a condição for verdadeira, execute o comando após o THEN*/
        size_t j = i + 4; // O próximo token após a condição

        while (j < tokens_classificados.size() && tokens_classificados[j].valor != "THEN") {
            j++;
        }

        if (j < tokens_classificados.size() && tokens_classificados[j].valor == "THEN") {
            /*Avança para o próximo comando após THEN*/
            j++;

            /*Verifica o tipo de comando após o THEN*/
            if (tokens_classificados[j].valor == "GOTO") {
                /*Processar GOTO*/
                if (!TratarGoto(tokens_classificados, j)) {
                    cout << "Erro" << endl;
                }
            } else if (ComandoAtribuicao(tokens_classificados, j)) {
                /*Processar atribuição LET*/
                string var_nome = tokens_classificados[j].valor;
                double valor = AnaliseEXPAritmetica(tokens_classificados, j + 2, tokens_classificados.size() - 1);
                tabela_simbolos[var_nome].valor = to_string(valor);
                //cout << "Atribuindo " << valor << " à variável " << var_nome << endl;
            } else if (ComandoPrint(tokens_classificados[j].valor)) {
                /*Processar PRINT*/
                cout << tokens_classificados[j + 1].valor << endl;
            }
        } else {
            cout << "Erro" << endl;
        }
    } else {
        /*Se a condição for falsa, pular a parte após o THEN, mas continuar nas próximas linhas*/
        //cout << "Condicao falsa, ignorando o restante da linha." << endl;

        /*Procurar pelo THEN e ignorar o que vem depois dele*/
        size_t j = i + 4;
        while (j < tokens_classificados.size() && tokens_classificados[j].valor != "THEN") {
            j++;
        }

        /* Se encontrou o THEN, pular até o final da linha*/
        if (j < tokens_classificados.size() && tokens_classificados[j].valor == "THEN") {
            while (j < tokens_classificados.size() && tokens_classificados[j].valor != "\n") {
                j++;
            }
        }
    }

    /* Avançar para o próximo token*/
    i++;  // Avançar para a próxima linha
}


void processarTokens(const vector<Token>& tokens_classificados){
    preencherTabelaDeLabels(tokens_classificados);
    size_t i = 0;
    bool finalizar = false;
    //for (size_t i = 0; i < tokens_classificados.size(); ++i) {
    while (i < tokens_classificados.size() && !finalizar) {  

        //DEPURAÇÃOOOOOOOOOOOOOOO
        //cout << "Executando linha: " << i << endl;

        if (tokens_classificados[i].valor == "DoisPontos") {
            // Ignorar o delimitador e continuar para o próximo comando
            continue;
        }

        /*Processar comando HALT*/
        if (tokens_classificados[i].tipo == COMANDO && tokens_classificados[i].valor == "HALT") {
            finalizar = true;
            break;
        } 

        //Processar o comando GOTO
        // Debug: exibir o valor do índice e do token atual
        //cout << "Processando índice: " << i << ", Token: " << tokens_classificados[i].valor << endl;
        /*
        if (tokens_classificados[i].tipo == COMANDO && tokens_classificados[i].valor == "GOTO") {
            TratarGoto(tokens_classificados, i);
            continue;
        } 
        */
       if (tokens_classificados[i].tipo == COMANDO && tokens_classificados[i].valor == "GOTO") {
            if (TratarGoto(tokens_classificados, i)) {
                //DEPURAÇÃOOOO
                //cout << "Pulo para linha: " << i << endl;
                continue; // Se o GOTO foi tratado, continua a partir da nova linha
            } else {
                //cout << "Label não encontrada";
                break;
            }
        }

        /*Processar o comando PRINT*/
        if (tokens_classificados[i].tipo == COMANDO && ComandoPrint(tokens_classificados[i].valor)) {
            // Verificar se o próximo token é uma string entre aspas
            if (i + 1 < tokens_classificados.size()) {
                /*Caso seja uma string */
                if (tokens_classificados[i + 1].tipo == STRING) {
                    string conteudo = tokens_classificados[i + 1].valor;
                    //Tirar as porras das aspas
                    conteudo = conteudo.substr(1, conteudo.size() - 2);
                    cout << conteudo << endl;
                    i++;
                }
                /* Capturar strings com múltiplos espaços ou tokens*/
                else if (tokens_classificados[i + 1].valor[0] == '"') {
                    string conteudo;
                    size_t j = i + 1;
                    /* Continua até encontrar a segunda aspa*/
                    while (j < tokens_classificados.size() && tokens_classificados[j].valor.back() != '"') {
                        conteudo += tokens_classificados[j].valor + " ";
                        j++;
                    }
                    /* Incluir o último token e remover as aspas*/
                    if (j < tokens_classificados.size()) {
                        conteudo += tokens_classificados[j].valor;
                        conteudo = conteudo.substr(1, conteudo.size() - 2);  // Remove as aspas
                        cout << conteudo << endl;
                        i = j;  // Avança o índice para pular todos os tokens da string
                    } else {
                        cout << "Erro";
                    }
                } 
                else if (tokens_classificados[i + 1].tipo == IDENTIFICADOR) {
                    string nome_var = tokens_classificados[i + 1].valor;
                    if (tabela_simbolos.find(nome_var) != tabela_simbolos.end()) {
                        if (tabela_simbolos[nome_var].tipo == NUMERO) 
                        {
                            cout << tabela_simbolos[nome_var].valor << endl;                            
                        } else if (tabela_simbolos[nome_var].tipo == STRING) {
                            cout << tabela_simbolos[nome_var].valor << endl;
                        }
                        
                    } else {
                        cout << "Erro";
                    }
                    i++; 
                }
                else {
                    cout << "Print sem string ou variavel.\n";
                }
            }
        
        }
        
        /* Processar ATRIBUIÇÂO de variáveis, já com OPERAÇÔES dessas variáveis*/
        if (ComandoAtribuicao(tokens_classificados, i)) {
            string nome_var = tokens_classificados[i].valor;
            i += 2; // Avança o índice para o valor atribuído
            
            if (i < tokens_classificados.size()) {
                if (tokens_classificados[i].tipo == STRING) {
                    string valor_string;
                    size_t j = i;
                    while (j < tokens_classificados.size() && tokens_classificados[j].tipo != COMANDO && tokens_classificados[j].valor.back() != '"') {
                        valor_string += tokens_classificados[j].valor + " ";
                        j++;
                    }
                    if (j < tokens_classificados.size()) {
                        valor_string += tokens_classificados[j].valor;
                        valor_string = valor_string.substr(1, valor_string.size() - 2);
                        tabela_simbolos[nome_var] = {nome_var, STRING, valor_string};
                        //cout <<  nome_var << " = " << valor_string << endl;
                        i = j;
                    } else {
                        cout << "Erro";
                    }
                }
                else {
                    size_t final_da_expressao = i;
                    while (final_da_expressao < tokens_classificados.size() && tokens_classificados[final_da_expressao].tipo != COMANDO && tokens_classificados[final_da_expressao].valor != "\n") {
                        final_da_expressao++;
                    }
                    double resultado = AnaliseEXPAritmetica(tokens_classificados, i, final_da_expressao - 1);
                    tabela_simbolos[nome_var] = {nome_var, NUMERO, to_string(resultado)};
                    //cout << nome_var << " = " << resultado << endl;
                    i = final_da_expressao - 1;
                }
            } else {
                cout << "Erro";
            }
        }

        /*Processar o comando INPUT*/
        if (tokens_classificados[i].tipo == COMANDO && ComandoInput(tokens_classificados[i].valor)) {
            //Vamos verificar se o próximo é um identificador
            if (i + 1 < tokens_classificados.size() && tokens_classificados[i + 1].tipo == IDENTIFICADOR) {
                string nome_var = tokens_classificados[i + 1].valor;
                string entrada;
                cout << "Entrada MyBasic:>> ";
                cin >> entrada;

                /*A entrada é numérica ? ou será uma stringgggggggggg*/
                if (isdigit(entrada[0])) {
                    tabela_simbolos[nome_var] = {nome_var, NUMERO, entrada};
                } else {
                    tabela_simbolos[nome_var] = {nome_var, STRING, entrada};
                }

                i++;
            } else {
                cout << "Erro" << endl;
            }
        }

        /*Processar comando IF <comparação> THEN <ação>*/
        if (tokens_classificados[i].tipo == COMANDO && ComandoIF(tokens_classificados[i].valor)) {
            AnalisaIF(tokens_classificados, i);
            continue;
        } else {
            //cout << "Erro no ifffff";
            //break;
        }
        

        
        /* Incrementa i ao final de cada iteração, exceto quando tratado dentro do bloco específico*/
        //if (tokens_classificados[i].tipo != COMANDO || tokens_classificados[i].valor != "GOTO") {
          //  i++;
        //}
        i++;
    }
}

/* Função para exibir a tabela de símbolos*/
void exibirTabelaSimbolos() {
    cout << "\nTabela de Simbolos:\n";
    for (const auto& [nome, simbolo] : tabela_simbolos) {
        cout << "Variavel: " << simbolo.nome << ", Tipo: " << tipoParaString(simbolo.tipo)
             << ", Valor: " << simbolo.valor << endl;
    }
}

/*Função para exibir a tabela de labels*/
void exibirTabelaLabels() {
    cout << "\nTabela de Labels:\n";
    for (const auto& [label, indice] : tabela_labels) {
        cout << "Label: " << label << ", Linha: " << indice << endl;
    }
}



int main() {

    /*
    Vamos primeiramente ler um arquivo .txt onde estará
    nosso código em Basic e colocar em um buffer/vetor
    */
   string arquvivo_txt = "programa.txt";
   ifstream inputFile(arquvivo_txt);

   string buffer;
   getline(inputFile, buffer, '\0');

   inputFile.close();

   cout << buffer << endl;

   cout << '\n';

   vector<string> tokens = dividirEmTokens(buffer);
   vector<Token> tokens_classificados;
   preencherVetorTokens(tokens, tokens_classificados);
   preencherTabelaDeLabels(tokens_classificados);

   cout << "Saida do programa.txt: \n";
   processarTokens(tokens_classificados);

   //Visualizar tabela para ver se está atribuindo corretamente:
   //exibirTabelaSimbolos();
   //exibirTabelaLabels();

}