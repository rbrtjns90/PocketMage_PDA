-- Compound Interest Calculator
-- Calculate future value of investment

principal = 1000
rate = 0.05
years = 10

print("Principal: $1000")
print("Rate: 5%")
print("Years: 10")

amount = principal * (1 + rate) ^ years
print("Future Value: $" .. string.format("%.2f", amount))

interest = amount - principal
print("Interest Earned: $" .. string.format("%.2f", interest))
