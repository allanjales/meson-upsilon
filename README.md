# Análise de dados em colisões pp

> Código com o objetivo de montar um histograma para massa invariante na colisão pp e indentificar os três estados da partícula méson-upsilon

## Importante

Embora o código também tenha sua parte para analisar os dados da colisão chumbo-chumbo (PbPb), elas não se encontram finalizadas!!!


## Arquivos extras necessários

Os dados a serem analisados serão dos arquivos:
* upsilonTree_2p76TeV_pp_data.root
* upsilonTree_2p76TeV_PbPb_data.root

Os arquivos extras e mais alguns adicionais podem ser encontrados [clicando aqui](https://drive.google.com/drive/folders/1DBPiqTYC6yJUPc9688nM8Azrw3UIo_YV)!
Note que os dados `PbPb` não estão sendo utilizados, pois o código está comentado nessa análise.


## Configuração de desenvolvimento

É necessário ter o [ROOT](https://root.cern.ch/root/html534/guides/users-guide/InstallandBuild.html), software do CERN, com o `cmake` instalado na sua máquina para rodar o código.

Vá na pasta onde se encontra os arquivos baixados pelo terminal e digite:

```sh
$ root -l -n
root[0] .x exercicio1.C
```

Um arquivo *.png aparecerá na pasta com o histograma gerado e também surgirá na sua tela.
