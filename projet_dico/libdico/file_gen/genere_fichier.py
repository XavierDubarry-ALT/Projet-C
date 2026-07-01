//PROJET JULIEN FISCHER ET XAVIER DUBARRY FIPA28 ENSTA
import random


with open("vocab.txt", encoding="utf-8") as f:
    vocab = [line.strip() for line in f if line.strip()] # rempli la liste vocab de tout les mots compris dans le fichier vocab.txt

lines_needed = 20000
uniq = set() # Initialise un dictionnaire (set)
with open("fr2000_20000.txt", "w", encoding="iso-8859-1") as out: #crée un fichier nommer fr2000_20000.txt passer à la variable out
    i = 0
    while i < lines_needed:
        mot = random.choice(vocab) # Sélectionne un mot random de la liste vocab et le met dans la varible mot
        try:
            out.write(mot + "\n") #essais d'écrire le mot dans le fichier et ajoute un retour à la ligne
        except:
            continue
        i+=1
        uniq.add(mot) # Ajoute le mod au dictionnaire
        
        if random.random()<0.1:# il y a 10 % de chance de créer un doublon dans le fichier final
            out.write(mot + "\n")
            i+=1

print(len(uniq)) # Renvoie le nombre de mot unique ajouter au fichier final (sans les doublons)
