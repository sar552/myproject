package main

import "fmt"

// Foydalanuvchi haqida struktura
type User struct {
    Name  string
    Age   int
    Email string
}

// Mahsulot haqida struktura
type Product struct {
    Name     string
    Price    float64
    Quantity int
}

func main() {
    user := User{
        Name:  "Sardorbek",
        Age:   19,
        Email: "sardorbekqanchibekov@gmail.com",
    }

    product := Product{
        Name:     "Laptop",
        Price:    1000.99,
        Quantity: 1,
    }

    fmt.Println("Foydalanuvchi:", user)
    fmt.Println("Mahsulot:", product)
}
