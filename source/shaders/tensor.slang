internal interface IShape<each I>
    where I : IInteger {
    uint element(expand each I);
}

internal struct Shape<each I> : IShape<expand each I> 
    where I : IInteger {

    private Tuple<expand each I> shape;
    internal int size;

    static void stepDimension<T : IInteger>(inout uint acc, T dimSize, T idx) {
        acc = acc * dimSize.toUInt() + idx.toUInt();
    }

    uint element(expand each I indices) {
        uint offset = 0;
        expand stepDimension(offset, each shape, each indices);

        return offset;
    }
}

public interface ITensor<T, each I>
    where I : IInteger {
    __subscript(expand each I args) -> T;
};

public struct Tensor<T, each I> : ITensor<T, expand each I>
    where I : IInteger {
    internal RWStructuredBuffer<T> data;
    internal Shape<expand each I> shape;
    
    public __subscript(expand each I indices) -> T {
        get { 
            return data[shape.element(expand each indices)];
        }
        set { 
            data[shape.element(expand each indices)] = newValue;
        }
    }
}

public extension<T, each I> Tensor<T, expand each I> : IArray<T>, IRWArray<T>
    where I : IInteger {
    public __subscript(uint i) -> T {
        get { return data[i]; }
        set { data[i] = newValue; }
    }

    public int getCount() {
        return shape.size;
    }
}

