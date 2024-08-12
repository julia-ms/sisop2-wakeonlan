#!/bin/bash

unset GTK_PATH

# Função para iniciar os contêineres
start_service() {
    docker-compose up -d "$1"
}

# Função para parar os contêineres
stop_service() {
    docker-compose stop "$1"
}

# Função para monitorar logs dos participantes
monitor_logs() {
    gnome-terminal -- bash -c "docker-compose logs -f participant1; exec bash"
    gnome-terminal -- bash -c "docker-compose logs -f participant2; exec bash"
    gnome-terminal -- bash -c "docker-compose logs -f participant3; exec bash"
    gnome-terminal -- bash -c "docker-compose logs -f participant4; exec bash"
}

# Iniciar o participant1 (Estação A)
start_service participant1

# Monitorar os logs dos contêineres dos participantes
monitor_logs

# Aguardar até que participant1 esteja pronto para se identificar como líder
sleep 15

# Iniciar o participant2 (Estação B)
start_service participant2

# Aguardar até que participant2 tenha tempo para reconhecer o líder
sleep 10

# Iniciar o participant3 (Estação C)
start_service participant3

# Aguardar até que participant3 tenha tempo para reconhecer o líder
sleep 10

# Iniciar o participant4 (Estação D)
start_service participant4

# Aguardar até que participant4 tenha tempo para reconhecer o líder
sleep 10

# Colocar participant1 para dormir (parar o líder original)
stop_service participant1

# Aguardar até que uma nova eleição seja realizada
sleep 15

# Colocar o novo líder para dormir (parar a nova estação líder)
# O novo líder deve ser o próximo da lista; se não tiver um critério específico, use participant2 como exemplo
stop_service participant2

# Aguardar até que uma nova eleição seja realizada
sleep 15

# Parar todos os contêineres
docker-compose down

echo "Contêineres gerenciados e parados com sucesso."
