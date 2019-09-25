#ifndef EX3_HASHMAP_H
#define EX3_HASHMAP_H

#include <stdlib.h>
#include <vector>
#include <math.h>
#include <algorithm>
#include <stdexcept>
#include <iostream>

const int INITCAPACITY = 16;
const double LOWER = 0.25;
const double UPPER = 0.75;
const int DEFUALT_SIZE = 0;

/**
 * Class represents HashMap
 * @tparam KeyT
 * @tparam ValueT
 */
template <typename KeyT, typename ValueT>
class HashMap
{
    using bucket = std::vector<std::pair<KeyT, ValueT>>; //each bucket is a vector
    using pair = std::pair<KeyT, ValueT>; //Represents a pair (Key,Value)

public:
    /**
     *Defualt Constructor
     */
    HashMap() : HashMap(LOWER, UPPER)
    {
    }
    /**
     *Constructor
     * @param lowerLoadFactor
     * @param upperLoadFactor
     */
    HashMap(double lowerLoadFactor, double upperLoadFactor) try :
            _hashTable(new bucket[INITCAPACITY]),
            _lowerLoadFactor(lowerLoadFactor), _upperLoadFactor(upperLoadFactor),
            _capacity(INITCAPACITY), _size(DEFUALT_SIZE)
    {
        if(lowerLoadFactor <= 0 || upperLoadFactor >= 1 || upperLoadFactor <= lowerLoadFactor)
        {
            throw std::invalid_argument("Invalid arguments");
        }
    }
    catch(std::bad_alloc& e)
    {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    /**
     *Constructor - gets 2 vectors and builds an hash map
     * @param keys
     * @param values
     */
    HashMap(const std::vector<KeyT> &keys, const std::vector<ValueT> &values): HashMap()
    {
        if(keys.size() != values.size())
        {
            delete[]_hashTable;
            throw std::invalid_argument("Invalid arguments");
        }

        for(int i = 0; i < keys.size(); ++i)
        {
            (*this)[keys[i]] = values[i];
        }

    }
    /**
     * Constructor - copy
     * @param other
     */
    HashMap(const HashMap &other): _hashTable(new bucket[other._capacity]),
         _lowerLoadFactor(other._lowerLoadFactor), _upperLoadFactor(other._upperLoadFactor)
            , _capacity(other._capacity), _size(other._size)
    {
        for(int i = 0; i < _capacity; i++)
        {
            _hashTable[i] = other._hashTable[i];
        }
    }
    /**
     * Constructor - move
     * @param other
     */
    HashMap(HashMap && other) noexcept
    {
        _capacity = other._capacity;
        _size = other._size;
        _lowerLoadFactor = other._lowerLoadFactor;
        _upperLoadFactor = other._upperLoadFactor;
        _hashTable = new bucket[INITCAPACITY];
        for(int i = 0; i < _capacity ; ++i)
        {
            _hashTable[i] = other._hashTable[i];
        }
        delete [] other._hashTable;
    }
    /**
     * Destructor
     */
    ~HashMap()
    {
        delete [] _hashTable;
    }

//********************************************************************************************
    /**
     * Returns number of elements in the hashTable
     */
    int size() const
    {
        return _size;
    }
    /**
     * Returns the max-capacity of the hashTable
     */
    int capacity() const
    {
        return _capacity;
    }
    /**
     * Returns load factor
     */
    double getLoadFactor() const
    {
        double load = static_cast<double>(_size) / static_cast<double>( _capacity);
        return load;
    }
    /**
     * Returns if the hashTable is empty
     */
    bool empty() const
    {
        return (_size == 0);
    }
    /**
     * Insert to table
     * @param key
     * @return
     */
    bool insert(const KeyT& key, const ValueT& value)
    {
        if(containsKey(key))
        {
            return false;
        }
        int index = _hashFunction(key);
        ++_size;
        double loadfactor = getLoadFactor();
        if(loadfactor > UPPER)
        {
            _rehash(_capacity * 2);
        }
        _hashTable[index].push_back(std::pair<KeyT, ValueT>(key, value));

        return true;

    }
    /**
     * Checks if key is contained
     * @param key
     * @return
     */
    bool containsKey(const KeyT key)
    {
        int index = _hashFunction(key);
        for(pair k: _hashTable[index])
        {
            if(key == k.first)
            {
                return true;
            }
        }
        return false;
    }
    /**
     * Returns value of a key
     * @param key
     * @return
     */
    ValueT& at(const KeyT key)
    {
        int index = _hashFunction(key);
        for(pair& k: _hashTable[index])
        {
            if(key == k.first)
            {
                return k.second;
            }
        }
        delete [] _hashTable;
        throw std::out_of_range("Invalid arguments");
    }
    /**
     * Resturns value of a key
     * @param key
     * @return
     */
    const ValueT& at(const KeyT key) const
    {
        int index = _hashFunction(key);
        for(pair k: _hashTable[index])
        {
            if(key == k.first)
            {
                return k.second;
            }
        }
        throw std::out_of_range("Invalid arguments");
    }
    /**
     * Erase pair in Hash Table
     * @param key
     * @return
     */
    bool erase(const KeyT key)
    {
        if(!containsKey(key))
        {
            return false;
        }
        else
        {
            int position = 0;
            int index = _hashFunction(key);
            for (pair &p: _hashTable[index])
            {
                if (p.first == key)
                {
                    _hashTable[index].erase(_hashTable[index].begin() + position);
                    --_size;
                    double loadfactor = getLoadFactor();
                    if (loadfactor <= LOWER)
                    {
                        _rehash(_capacity / 2);
                    }
                    return true;
                }
                position++;
            }
        }
    }
    /**
     * Returns Bucket size
     * @param key
     * @return
     */
    int bucketSize(const KeyT key)
    {
        if(!containsKey(key))
        {
            throw std::out_of_range("Invalid arguments");
        }
        int index = _hashFunction(key);
        return static_cast<int>(_hashTable[index].size());
    }
    /**
     * Clears the table
     */
    void clear()
    {
        for(int i = 0; i < _capacity; ++i)
        {
            _hashTable[i].clear();
        }
        _size = DEFUALT_SIZE;
    }
    //********************************Operators**************************************************
    /**
     *Operator overloading
     * @param other
     * @return
     */
    HashMap&operator=(HashMap other)
    {
        std::swap(_lowerLoadFactor, other._lowerLoadFactor);
        std::swap(_upperLoadFactor, other._upperLoadFactor);
        std::swap(_capacity, other._capacity);
        std::swap(_size, other._size);
        std::swap(_hashTable, other._hashTable);
        return *this;
    }
    /**
     *Operator overloading
     * @param key
     * @return
     */
    ValueT&operator[](const KeyT key) noexcept
    {
        int i = _hashFunction(key);
        if(containsKey(key))
        {
            return _hashTable[i].front().second;
        }
        else
        {
            ++_size;
            if(getLoadFactor() >= UPPER)
            {
                _rehash(_capacity * 2);
            }
            _hashTable[i].push_back(std::pair<KeyT, ValueT>(key, ValueT()));
        }
        return _hashTable[i].back().second;
    }
    /**
     *Operator overloading
     * @param key
     * @return
     */
    ValueT&operator[](const KeyT key) const noexcept
    {
        if(containsKey(key))
        {
            return at(key);
        }
    }
    /**
     *Operator overloading
     * @param other
     * @return
     */
    bool operator==(const HashMap& other)
    {
        if(size() != other.size() && capacity()!=other.capacity())
        {
            return false;
        }

        for(int i = 0; i < size(); i++)
        {
            std::sort(_hashTable[i].begin(), _hashTable[i].end());
            std::sort(other._hashTable[i].begin(), other._hashTable[i].end());
            if(_hashTable[i]!= other._hashTable[i])
            {
                return false;
            }
        }
        return true;
    }
    /**
     *Operator overloading
     * @param other
     * @return
     */
    bool operator!=(const HashMap& other)
    {
        if(size() != other.size() && capacity() != other.capacity())
        {
            return true;
        }

        for(int i = 0; i < size(); i++)
        {
            std::sort(_hashTable[i].begin(), _hashTable[i].end());
            std::sort(other._hashTable[i].begin(), other._hashTable[i].end());
            if(_hashTable[i] == other._hashTable[i])
            {
                return false;
            }
        }
        return true;
    }
    //********************************Iterator*************************************************
    /**
     *Represents an Iterator for HashMap
     */
    class iterator
    {
        friend class HashMap;
    public:
        /**
         *
         * @return
         */
        iterator &operator++()
        {
            _nextPosition();
            return *this;
        }
        /**
         *
         * @return
         */
        const iterator operator++(int)
        {
            iterator temp = *this;
            _nextPosition();
            return *temp;
        }
        /**
         *
         * @return
         */
        pair& operator*() const
        {
            return (_map->_hashTable[_bucketIndex][_pairIndex]);
        }
        /**
         *
         * @return
         */
        pair* operator->() const
        {
            return &(_map->_hashTable[_bucketIndex][_pairIndex]);
        }
        /**
         *
         * @param other
         * @return
         */
        bool operator==(const iterator &other)
        {
            return (*_map == *(other._map)) && (_bucketIndex == other._bucketIndex)
            && (_pairIndex == other._pairIndex);
        }
        /**
         *
         * @param other
         * @return
         */
        bool operator!=(const iterator &other)
        {
            return (*_map != *(other._map)) || (_bucketIndex != other._bucketIndex)
            || (_pairIndex != other._pairIndex);
        }


    private:

        HashMap* _map;
        int _bucketIndex;
        int _pairIndex;
        /**
         *
         * @param other
         * @param bucketIndex
         * @param pairIndex
         */
        explicit iterator(HashMap* other = nullptr, int bucketIndex = 0, int pairIndex = -1):
                    _map(other), _bucketIndex(bucketIndex), _pairIndex(pairIndex)
        {
            _nextPosition();
        }
        /**
         *
         */
        void _nextPosition()
        {
            while (_bucketIndex < _map->capacity())
            {
                if (_pairIndex + 1 < static_cast<int>(_map->_hashTable[_bucketIndex].size()))
                {
                    _pairIndex++;
                    return;
                }
                else
                {
                    _pairIndex = -1;
                    _bucketIndex++;
                }
            }
            _pairIndex = 0;
        }

    };

    /**
     *
     * @return
     */
    iterator cbegin()
    {
        return iterator(this);
    }
    /**
     *
     * @return
     */
    iterator begin()
    {
        return iterator(this);
    }
    /**
     *
     * @return
     */
    iterator cend()
    {
        return iterator(this, capacity());
    }
    /**
     *
     * @return
     */
    iterator end()
    {
        return iterator(this, capacity());
    }

private:

    bucket* _hashTable;
    double _lowerLoadFactor;
    double _upperLoadFactor;
    int _capacity;
    int _size;
    /**
     * Returns hash value
     * @param key
     * @return
     */
    int _hashFunction(const KeyT &key)
    {
        return (std::hash<KeyT>{}(key)&(_capacity - 1));
    }
    /**
     * Rehashing the table
     * @param newCapacity
     */
    void _rehash(const int& newCapacity)
    {
        int oldCapacity = _capacity;
        _capacity = newCapacity;
        auto * newTable = new bucket[_capacity];
        for(int index = 0; index < oldCapacity ; ++index)
        {
            for(pair& p: _hashTable[index])
            {
                KeyT key = p.first;
                int newIndex = _hashFunction(key);
                newTable[newIndex].push_back(std::move(p));
            }
        }
        delete []_hashTable;
        _hashTable = newTable;
    }
    /**
     * Next power of two
     * @param capacity
     * @return
     */
    int _nextPower2(int& capacity)
    {
        int counter = 0;
        while (capacity > 0)
        {
            counter++;
            capacity = capacity >> 1;
        }
        return (1 << counter);
    }
};

#endif //EX3_HASHMAP_H
