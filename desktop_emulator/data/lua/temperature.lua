-- Temperature Conversion
-- Convert Fahrenheit to Celsius

fahrenheit = 98.6
print("Fahrenheit: " .. fahrenheit .. "°F")

celsius = (fahrenheit - 32) * 5 / 9
print("Celsius: " .. string.format("%.1f", celsius) .. "°C")
