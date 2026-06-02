import random

# Liste de 2000 mots français fréquents (extrait de sources publiques)
with open("vocab.txt", encoding="utf-8") as f:
    vocab = [line.strip() for line in f if line.strip()]

lines_needed = 20000
uniq = set()
with open("fr2000_20000.txt", "w", encoding="iso-8859-1") as out:
    i = 0
    while i < lines_needed:
        mot = random.choice(vocab)
        try:
            out.write(mot + "\n")
        except:
            continue
        i+=1
        uniq.add(mot)
        if random.random()<0.1:
            out.write(mot + "\n")
            i+=1

print(len(uniq))
