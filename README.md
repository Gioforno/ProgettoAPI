# ProgettoAPI

Prova finale di Algoritmi e Principi dell'Informatica presso il Politecnico di Milano.

Voto 24/30, tempo necessario circa 2 settimane di lavoro continuativo.

## cartelle 
La consegna si trova nella cartella specifica/.

Sono presenti dei file contenenti vari casi da testare in test_case/ con il relativo output per verificare la correttezza in output/.


## esecuzione
Per eseguire il progetto e trovare le differenze con l'output giusto:
``` gcc -Wall -Werror -std=gnu11 -O2 -lm test.c -o test
``` ./programma < test_case/test1 > out1
``` diff out1 output/test1
