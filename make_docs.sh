#!/bin/zsh

# Skrypt do generowania dokumentacji Doxygen i otwarcia jej w przeglądarce

echo "Generowanie dokumentacji Doxygen..."
doxygen Doxyfile

if [ $? -eq 0 ]; then
    echo "Dokumentacja wygenerowana poprawnie."
    if [ -f "docs/html/index.html" ]; then
        echo "Otwieranie dokumentacji w przeglądarce..."
        google-chrome docs/html/index.html &
    else
        echo "Błąd: Plik docs/html/index.html nie istnieje."
    fi
else
    echo "Błąd podczas generowania dokumentacji."
fi
