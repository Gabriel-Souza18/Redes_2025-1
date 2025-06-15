#!/bin/bash

# Configurações
REQUESTS=100
CONCURRENCY=10
TEST_FILE="test.html"

# Lista de servidores e portas 
SERVERS=(
    "iterativo 2020"
    "concorrente 2021" 
    "thread 2022"
    "thread_e_fila 2023"
)

# Função para encerrar servidor
kill_server() {
    pkill -f "./$1" 2>/dev/null
    sleep 0.5  # Pequena espera para liberar porta
}

# Prepara arquivo de teste
mkdir -p www
echo "<html><body><h1>Teste de Performance</h1></body></html>" > "www/$TEST_FILE"

# Loop de testes
for server_info in "${SERVERS[@]}"; do
    read -r server_name port <<< "$server_info"
    
    echo -e "\n\033[1;36m=== Testando $server_name (porta $port) ===\033[0m"
    
    kill_server "$server_name"
    
    # Inicia servidor
    ./"$server_name" &
    SERVER_PID=$!
    sleep 1  # Espera inicialização
    
    # Executa teste
    ab -n $REQUESTS -c $CONCURRENCY "http://localhost:$port/$TEST_FILE" 2>&1 | \
    grep -E --color=always 'Requests per second|Time per request|Transfer rate|Failed requests|^Complete requests'
    
    # Encerra
    kill_server "$server_name"
done

echo -e "\n\033[1;32mTodos os testes concluídos!\033[0m"