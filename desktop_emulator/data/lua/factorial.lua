-- Factorial Calculator
-- Calculate n! using a for loop

n = 5
print("Calculating " .. n .. "!")

fact = 1
for i = 1, n do
  fact = fact * i
end

print(n .. "! = " .. fact)
