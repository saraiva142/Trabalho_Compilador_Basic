# Interpretador De Texto Basic 🤖
Quando tirei dúvida com o professor sobre o trabalho, entendi por ele que não erra preciso fazer um processo 
de tokerização, visto que o trabalho é um interpretador de textos e não um compilador.
Porém como eu ja tinha feito esse processo e estava funcionando bem, decidi integrar ao meu trabalho, isso pode prejudicar o 
tempo de execução do programa porém acho que dá mais segurança. (Se é que tem alguma segurança em um interpretador)
O programa está sendo baseado em uma série de autômatos (porém implementado com, for's e while's, como se fossem entradas e saídas), como na imagem :

<img width="576" alt="LFA_NDE" src="https://github.com/user-attachments/assets/540985f4-3a38-4374-8599-a145b9b0725d">

## Funicionalidades:
O meu interpretador é capaz de processar os seguintes comandos:
* Atribuição LET
  * `LET Variável = INTEIRO`
  * `LET Variável = CHAR`
* PRINT
  * `PRINT "STRING"`
  * `PRINT Variáve`
* INPUT
  * `INPUT Variável`
* GOTO
  * `GOTO Label`
* IF
  * IF <condição> THEN <ação> :
    * `IF Variável <comparação> Variável THEN Atribuição Variável`
    * `IF Variável <comparação> Variável THEN PRINT Variável/STRING`
    * `IF Variável <comparação> Variável THEN GOTO Label`
* DOIS PONTOS
  * `<comando>` `:` `<comando>`
  * `INPUT B` `:` `PRINT B`
* HALT
  * `Label HALT`

## Obs
Com a implementação da tabela de símbolos e de labels, no final do código pode ser descomentada para debugging 
`exibirTabelaSimbolos();`
`exibirTabelaLabels();`

> 🚧 Projeto em construção 🚧
