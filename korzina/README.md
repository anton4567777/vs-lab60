# Программа для ведения счета на складе
## Структура базы данных
id | Название | Цена | Количество

## Что делает программа?
- Добавляет
- Удаляет
- Выводит список

## Установка программы
```bash
git clone https://github.com/anton4567777/vs-lab60

gcc -c sqlite3.c -o sqlite3.o

g++ main.cpp sqlite3.o -lpthread -ldl -o shop
```

