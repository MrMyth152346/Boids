class Vector
{
public:
    float X;
    float Y;

    double Magnitude()
    {
        return SDL_sqrt(SDL_pow(this->X, 2.0f) + SDL_pow(this->Y, 2.0f));
    }

    Vector Unit()
    {
        double magnitude = this->Magnitude();

        Vector vector_unit = *this;

        if (magnitude != 0)
            return vector_unit / magnitude;
        else
            return vector_unit;
    }

    Vector operator+(Vector vector)
    {
        Vector newVector = *this;
        newVector.X += vector.X;
        newVector.Y += vector.Y;

        return newVector;
    }


    Vector operator-(Vector vector)
    {
        Vector newVector = *this;
        newVector.X -= vector.X;
        newVector.Y -= vector.Y;

        return newVector;
    }

    Vector operator*(double number)
    {
        Vector newVector = *this;
        newVector.X *= number;
        newVector.Y *= number;

        return newVector;
    }

    Vector operator/(double number)
    {
        Vector newVector = *this;
        newVector.X /= number;
        newVector.Y /= number;

        return newVector;
    }

    void operator+=(Vector vector)
    {
        this->X -= vector.X;
        this->Y -= vector.Y;
    }

    Vector(double X, double Y)
    {
        this->X = X;
        this->Y = Y;
    }
    
    Vector(){};
};