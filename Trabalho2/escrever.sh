#!/bin/bash

# Configurações
REQUESTS_VALUES=(100 250 500 1000 2000 )
CONCURRENCY_VALUES=(1 10 25 50 100)
TEST_FILE="image.jpg"
OUTPUT_DIR="test_results"
LONG_TIMEOUT=60  
SHORT_TIMEOUT=10   

# Lista de servidores e portas 
SERVERS=(
    "iterativo 2020"
    "concorrente 2021" 
    "thread 2022"
    "thread_e_fila 2023"
)

# Métricas a serem coletadas
METRICS=(
    "Requests_per_second"
    "Time_per_request_mean"
    "Time_per_request_concurrent"
    "Transfer_rate"
    "Failed_requests"
)

# Preparar diretório de saída
mkdir -p "$OUTPUT_DIR"

# Função para coletar métricas
collect_metrics() {
    local output="$1"
    echo "$output" | awk '
        /Requests per second/ {rps=$4}
        /Time per request.*mean)/ {tpr_mean=$4}
        /Time per request.*mean, across/ {tpr_conc=$4}
        /Transfer rate/ {trans=$3}
        /Failed requests/ {fail=$3}
        END {
            printf "%s,%s,%s,%s,%s", rps, tpr_mean, tpr_conc, trans, fail
        }
    '
}

# Loop por concorrências
for concurrency in "${CONCURRENCY_VALUES[@]}"; do
    echo -e "\n\033[1;35m====== TESTANDO CONCORRÊNCIA: $concurrency ======\033[0m"
    
    # Preparar arquivos de saída para esta concorrência
    for metric in "${METRICS[@]}"; do
        echo -n "Requisicoes" > "$OUTPUT_DIR/${metric}_c${concurrency}.txt"
        for server_info in "${SERVERS[@]}"; do
            read -r server_name _ <<< "$server_info"
            echo -n -e "\t$server_name" >> "$OUTPUT_DIR/${metric}_c${concurrency}.txt"
        done
        echo "" >> "$OUTPUT_DIR/${metric}_c${concurrency}.txt"
    done

    # Loop por requisições
    for requests in "${REQUESTS_VALUES[@]}"; do
        echo -e "\n\033[1;36m===== REQUISIÇÕES: $requests =====\033[0m"
        
        declare -A results
        
        # Loop por servidores
        for server_info in "${SERVERS[@]}"; do
            read -r server_name port <<< "$server_info"
            
            echo -e "\n\033[1;33m=== Servidor: $server_name (porta $port) ===\033[0m"
            
            # Limpeza prévia agressiva
            pkill -9 -x "$server_name" 2>/dev/null
            kill -9 $(lsof -ti :$port) 2>/dev/null
            
            # Inicia servidor
            echo "Iniciando servidor..."
            ./"$server_name" &
            SERVER_PID=$!
            
            # Espera inicialização
            sleep 5
            
            # Verifica se está rodando
            if ! ps -p $SERVER_PID > /dev/null; then
                echo -e "\033[1;31mErro: Servidor não iniciou!\033[0m"
                results["${server_name}_rps"]="FAIL"
                results["${server_name}_tpr_mean"]="FAIL"
                results["${server_name}_tpr_conc"]="FAIL"
                results["${server_name}_trans"]="FAIL"
                results["${server_name}_fail"]="FAIL"
                continue
            fi
            
            # Executa teste
            echo "Executando teste com ab (concorrência: $concurrency, requisições: $requests)..."
            AB_OUTPUT=$(ab -n $requests -c $concurrency "http://localhost:$port/$TEST_FILE" 2>&1)
            echo "$AB_OUTPUT" > "$OUTPUT_DIR/${server_name}_c${concurrency}_n${requests}.log"
            
            # Processa métricas
            metrics=$(collect_metrics "$AB_OUTPUT")
            IFS=',' read -r rps tpr_mean tpr_conc trans fail <<< "$metrics"
            
            results["${server_name}_rps"]="$rps"
            results["${server_name}_tpr_mean"]="$tpr_mean"
            results["${server_name}_tpr_conc"]="$tpr_conc"
            results["${server_name}_trans"]="$trans"
            results["${server_name}_fail"]="$fail"
            
            echo "Métricas coletadas:"
            echo "  RPS: $rps"
            echo "  TPR Mean: $tpr_mean"
            echo "  TPR Conc: $tpr_conc"
            echo "  Transfer: $trans"
            echo "  Failed: $fail"
            
            # Encerra servidor
            echo "Encerrando servidor..."
            kill -9 $SERVER_PID 2>/dev/null
            
            # Espera entre servidores
            echo -e "\033[1;35mEsperando $SHORT_TIMEOUT segundos antes do próximo servidor...\033[0m"
            sleep $SHORT_TIMEOUT
        done
        
        # Escreve resultados nos arquivos para esta concorrência
        for metric in "${METRICS[@]}"; do
            echo -n "$requests" >> "$OUTPUT_DIR/${metric}_c${concurrency}.txt"
            
            for server_info in "${SERVERS[@]}"; do
                read -r server_name _ <<< "$server_info"
                
                case $metric in
                    "Requests_per_second") key="${server_name}_rps" ;;
                    "Time_per_request_mean") key="${server_name}_tpr_mean" ;;
                    "Time_per_request_concurrent") key="${server_name}_tpr_conc" ;;
                    "Transfer_rate") key="${server_name}_trans" ;;
                    "Failed_requests") key="${server_name}_fail" ;;
                esac
                
                value="${results[$key]}"
                echo -n -e "\t$value" >> "$OUTPUT_DIR/${metric}_c${concurrency}.txt"
            done
            
            echo "" >> "$OUTPUT_DIR/${metric}_c${concurrency}.txt"
        done
        echo -e "\033[1;35mEsperando $LONG_TIMEOUT segundos...\033[0m"
           sleep $LONG_TIMEOUT
    done
    
    # Espera longa entre mudanças de concorrência
    echo -e "\033[1;35mEsperando $LONG_TIMEOUT segundos antes da próxima concorrência...\033[0m"
    sleep $LONG_TIMEOUT
done

# Resumo final
echo -e "\n\033[1;32m===== TESTES CONCLUÍDOS! =====\033[0m"
echo "Arquivos de resultados:"
find "$OUTPUT_DIR" -type f | sort
echo "Use 'column -t -s $'\t' <arquivo>' para visualizar os resultados"