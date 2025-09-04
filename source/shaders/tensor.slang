public interface ITensor<T, let D0:int, let D1:int = 1, let D2:int = 1, let D3:int = 1>
{
    __subscript(int i) -> T;
    __subscript(int i, int j) -> T;
    __subscript(int i, int j, int k) -> T;
    __subscript(int i, int j, int k, int l) -> T;

    int getCount();
}

// Fixed-size tensor up to 4D
public struct Tensor<T, let D0:int, let D1:int = 1, let D2:int = 1, let D3:int = 1>
    : ITensor<T, D0, D1, D2, D3>
{
    static const int Count = D0 * D1 * D2 * D3;
    T data[Count];

    int dim0() { return D0; }  
    int dim1() { return D1; }
    int dim2() { return D2; }  
    int dim3() { return D3; }

    // Row-major indexers
    static int idx(int i) { return i; }
    static int idx(int i, int j) { return i * D1 + j; }
    static int idx(int i, int j, int k) { return (i * D1 + j) * D2 + k; }
    static int idx(int i, int j, int k, int l) { return ((i * D1 + j) * D2 + k) * D3 + l; }

    // 1D
    public __subscript(int i) -> T {
        get { return data[idx(i)]; }
        set { data[idx(i)] = newValue; }
    }
    // 2D
    public __subscript(int i, int j) -> T {
        get { return data[idx(i, j)]; }
        set { data[idx(i, j)] = newValue; }
    }
    // 3D
    public __subscript(int i, int j, int k) -> T {
        get { return data[idx(i, j, k)]; }
        set { data[idx(i, j, k)] = newValue; }
    }
    // 4D
    public __subscript(int i, int j, int k, int l) -> T {
        get { return data[idx(i, j, k, l)]; }
        set { data[idx(i, j, k, l)] = newValue; }
    }
}

public extension<T, let D0:int, let D1:int, let D2:int, let D3:int>
Tensor<T, D0, D1, D2, D3> : IArray<T>, IRWArray<T>
{
    public __subscript(uint i) -> T {
        get { return this.data[i]; }
        set { this.data[i] = newValue; }
    }

    public int getCount() { return Count; }
}



public interface ITensorBuffer<T, each I> where I : IInteger {
    __subscript(expand each I args) -> T;
}

internal interface IShape<each I> where I : IInteger {
    uint element(expand each I);
}

internal struct Shape<each I> : IShape<expand each I> where I : IInteger {
    private Tuple<expand each I> shape;
    int size;

    static void stepDimension<T : IInteger>(inout uint acc, T dimSize, T idx) {
        acc = acc * dimSize.toUInt() + idx.toUInt();
    }

    uint element(expand each I indices) {
        uint offset = 0;
        expand stepDimension(offset, each shape, each indices);
        return offset;
    }
}



// TensorBuffer backed by StructuredBuffer
public struct TensorBuffer<T, each I> : ITensorBuffer<T, expand each I>
where I : IInteger {
    internal StructuredBuffer<T> data;
    internal Shape<expand each I>  shape;

    public __subscript(expand each I indices) -> T {
        get { return this.data[this.shape.element(expand each indices)]; }
    }
}

public extension<T, each I> 
TensorBuffer<T, expand each I> : IArray<T>
where I : IInteger {
    public __subscript(uint i) -> T {
        get { return this.data[i]; }
    }

    public int getCount() { return this.shape.size; }
}



// Read-write TensorBuffer backed by RWStructuredBuffer
public struct RWTensorBuffer<T, each I> : ITensorBuffer<T, expand each I>
where I : IInteger {
    internal RWStructuredBuffer<T> data;
    internal Shape<expand each I>  shape;

    public __subscript(expand each I indices) -> T {
        get { return this.data[this.shape.element(expand each indices)]; }
        set { data[this.shape.element(expand each indices)] = newValue; }
    }
}

public extension<T, each I>
RWTensorBuffer<T, expand each I> : IRWArray<T>
where I : IInteger {
    public __subscript(uint i) -> T {
        get { return this.data[i]; }
        set { data[i] = newValue; }
    }

    public int getCount() { return this.shape.size; }
}

