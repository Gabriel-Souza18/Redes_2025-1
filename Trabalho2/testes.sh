#!/bin/bash

# Configurações
REQUESTS=1000
CONCURRENCY=10
TEST_FILE="image.jpg"

# Lista de servidores e portas 
SERVERS=(
    "iterativo 2020"
    "concorrente 2021" 
    "thread 2022"
    "thread_e_fila 2023"
)

# Função robusta para encerrar servidor
kill_server() {
    local server_name=$1
    local port=$2
    
    # Mata por nome do executável
    pkill -x "$server_name" 2>/dev/null
    
    # Mata por porta
    local pid=$(lsof -ti :$port)
    if [ ! -z "$pid" ]; then
        kill -9 $pid 2>/dev/null
    fi
    
    sleep 0.5  # Pequena espera para liberar porta
}

# Loop de testes
for server_info in "${SERVERS[@]}"; do
    read -r server_name port <<< "$server_info"
    
    echo -e "\n\033[1;36m=== Testando $server_name (porta $port) ===\033[0m"
    
    # Encerra qualquer instância prévia
    kill_server "$server_name" "$port"
    
    # Inicia servidor gravando PID
    ./"$server_name" &
    SERVER_PID=$!

    
    sleep 1  # Espera inicialização
    
    # Verifica se o servidor está rodando
    if ! ps -p $SERVER_PID > /dev/null; then
        echo -e "\033[1;31mErro: Servidor não iniciou corretamente!\033[0m"
        continue
    fi
    
    # Executa teste
    echo -e "\033[1;33mExecutando teste...\033[0m"
    ab -n $REQUESTS -c $CONCURRENCY "http://localhost:$port/$TEST_FILE" 2>&1 | \
    grep -E --color=always 'Total transferred|Transfer rate|Requests per second|Complete requests|Failed requests' || \
    
    # Encerra servidor
    kill_server "$server_name" "$port"
    
    echo -e "\033[1;32mTeste concluído!\033[0m"
done

echo -e "\n\033[1;32mTodos os testes concluídos!\033[0m"