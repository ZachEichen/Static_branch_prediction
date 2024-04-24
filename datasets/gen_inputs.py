from numpy.random import randint

def main(): 
    print(randint(0,30))
    for i in range(100): 
        print(randint(-256,256))

if __name__ == "__main__":
    main()