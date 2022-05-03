from collections import defaultdict
input_path = "./d2.log"
skip = 0
d = defaultdict(lambda: 0)
count = 0
f = open("./d2.log")
lines = list(f.readlines())
# we only care about the stable state
lines = lines[skip:]
for l in lines:
    count += 1
    tokens = l.split(" ")
    for token in tokens:
        try:
            k, v = token.split(":")
        except:
            continue
        d[k] += float(v)

for k, v in d.items():
    print(f"{k} {v/count}")
print(count)
            
