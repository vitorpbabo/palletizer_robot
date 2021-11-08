# Robô palletizer
Robô palletizer capaz de reconhecer a cor de uma peça (vermelha, verde, azul, preta) e depositá-la na caixa da cor respetiva. São utilizados 3 servos (MG996R), sendo um deles de rotação continua. É ainda utilizado um electroíman para agarrar as peças e um sensor de cor (TCS3200).

Não foram utilizadas quaisquer bibliotecas existentes, apenas as funcionalidades base do ATMEGA328 como rotinas de interrupção, conversor AD, EEPROM, etc.

**Esquemático da interface**

<img src="https://github.com/vitorpbabo/palletizer_robot/blob/main/images/interface_schematic.png" width="500" height="400">

