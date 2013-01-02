
int
ipow(int xx, int nn)
{
    int yy = 1;

    for (int ii = 0; ii < nn; ++ii) {
        yy = yy * xx;
    }

    return yy;
}
