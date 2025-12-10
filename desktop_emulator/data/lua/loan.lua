-- Loan Payment Calculator
-- Calculate monthly payment for a loan

P = 10000
annual_rate = 0.05
years = 5

print("Loan Amount: $10,000")
print("Annual Rate: 5%")
print("Term: 5 years")

r = annual_rate / 12
n = years * 12

payment = P * r * (1+r)^n / ((1+r)^n - 1)
print("Monthly Payment: $" .. string.format("%.2f", payment))

total = payment * n
print("Total Paid: $" .. string.format("%.2f", total))

interest = total - P
print("Total Interest: $" .. string.format("%.2f", interest))
