import math
# Calculate e^x values
values = [math.exp(x) for x in [-7, -8, -9, -10, -13]]

# Calculate sum for normalization
total = sum(values)

# Normalize by dividing each value by sum
normalized = [v/total for v in values]
p  = [0.68,0.82,0.79,0.82,0.83]
delta = 0.05
n = 300
#to 3 decimal places
print([round(v,3) for v in normalized])
def CHB(risk,p):
    return risk + math.sqrt((math.log(1/p) + math.log(1 / delta)) / (2 * n))
for i in range(5):
    print(round(CHB(1 -p[i],normalized[i]),3))