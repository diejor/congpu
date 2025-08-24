interface IShape<each I>
    where I : IInteger {
    uint element(expand each I);
}

struct Shape<each I> : IShape<expand each I> 
    where I : IInteger {
    int size;
    Tuple<expand each I> shape;

    static void stepDimension<T : IInteger>(inout uint acc, T dimSize, T idx) {
        acc = acc * dimSize.toUInt() + idx.toUInt();
    }

    uint element(expand each I indices) {
        uint offset = 0;
        expand stepDimension(offset, each shape, each indices);

        return offset;
    }
}

interface ITensor<T, each I>
    where I : IInteger {
    __subscript(expand each I args) -> T;
};

struct Tensor<T, each I> : ITensor<T, expand each I>
    where I : IInteger {
    RWStructuredBuffer<T> data;
    Shape<expand each I> shape;
    
    __subscript(expand each I indices) -> T {
        get { 
            return data[shape.element(expand each indices)];
        }
        set { 
            data[shape.element(expand each indices)] = newValue;
        }
    }
}

extension<T, each I> Tensor<T, expand each I> : IArray<T>, IRWArray<T>
    where I : IInteger
{
    __subscript(uint i) -> T
    {
        get { return data[i]; }
        set { data[i] = newValue; }
    }

    static void mulDim<TI : IInteger>(inout uint acc, TI d) { acc *= d.toUInt(); }

    int getCount()
    {
        uint n = 1;
        expand mulDim(n, each shape.shape);
        return int(n);
    }
}

