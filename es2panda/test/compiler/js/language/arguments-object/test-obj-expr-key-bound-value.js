let number = {
    1e6   : '1,000,000',
    1e-6 : '0.000001',
    1e7   : '10,000,000',
    1e-7 : '1e-7',
    1e18 : '1,000,000,000,000,000,000',
    1e19 : '10,000,000,000,000,000,000',
    1e20 : '100,000,000,000,000,000,000',
    1e21 : '1,000,000,000,000,000,000,000',
}

print(number[1e6]);
print(number[1e-6]);
print(number[1e7]);
print(number[1e-7]);
print(number[1e18]);
print(number[1e19]);
print(number[1e20]);
print(number[1e21]);
