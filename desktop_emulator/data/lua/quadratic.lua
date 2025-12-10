-- Quadratic Formula
-- Solve ax^2 + bx + c = 0

a = 1
b = -5
c = 6
print("Equation: x^2 - 5x + 6 = 0")

discriminant = b^2 - 4*a*c
print("Discriminant: " .. discriminant)

x1 = (-b + math.sqrt(discriminant)) / (2*a)
x2 = (-b - math.sqrt(discriminant)) / (2*a)

print("Solution x1: " .. x1)
print("Solution x2: " .. x2)
