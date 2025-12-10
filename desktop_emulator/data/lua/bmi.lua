-- BMI Calculator
-- Calculate Body Mass Index

weight_kg = 70
height_m = 1.75

print("Weight: " .. weight_kg .. " kg")
print("Height: " .. height_m .. " m")

bmi = weight_kg / (height_m ^ 2)
print("BMI: " .. string.format("%.1f", bmi))

print("")
if bmi < 18.5 then
  print("Status: Underweight")
elseif bmi < 25 then
  print("Status: Normal")
elseif bmi < 30 then
  print("Status: Overweight")
else
  print("Status: Obese")
end
