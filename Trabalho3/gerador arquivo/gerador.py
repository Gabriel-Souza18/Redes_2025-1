import random
import string
tam = 100000 # Bytes

arq = open(f'arquivo_{tam}.txt', "w")
for i in range(tam):
    arq.write( random.choice(string.ascii_letters))
    
arq.close()
print("Arquivo gerado com " + str(tam)+ " bytes de tamanho")