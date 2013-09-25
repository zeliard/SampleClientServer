#pragma once

#include <assert.h>


class ABCircularBuffer
{
public:
    ABCircularBuffer(int capacity) 
		: mBuffer(NULL), mIndexA(0), mSizeA(0), mIndexB(0), mSizeB(0), mCapacity(capacity), mIndexReserve(0), mSizeReserve(0)
    {
		mBuffer = new char[mCapacity] ;
	}

    ~ABCircularBuffer()
    {
		delete [] mBuffer ;
    }


	/// �տ��� Reserve�س��� Commit ���ѻ��¿��� Reserver���Ϸ��� null�����Ѵ�
    char* Reserve(int size, OUT int& reserved)
    {
        /// ���� B������ ������ �ִٸ� ����
        if ( mSizeB > 0 )
        {
            int freespace = GetBFreeSpace() ;

            if (size < freespace) 
				freespace = size ;

            if (freespace == 0)
				return nullptr ;

            mSizeReserve = freespace ;
            reserved = freespace ;
            mIndexReserve = mIndexB + mSizeB ;

            return mBuffer + mIndexReserve ;
        }
		/// A���� ������ ������� ����
        else
        {
	        // Block b does not exist, so we can check if the space AFTER a is bigger than the space
            // before A, and allocate the bigger one.
			
            int freespace = GetSpaceAfterA() ;

			/// A�������� ���ʿ� ������ �� ���� ���
            if (freespace >= mIndexA)
            {
                if (freespace == 0) 
					return nullptr ;

                if (size < freespace)
					freespace = size ;

                mSizeReserve = freespace ;
                reserved = freespace ;
                mIndexReserve = mIndexA + mSizeA ;

                return mBuffer + mIndexReserve ;
            }
			/// A�������� ���ʿ� ������ �� ���� ���
            else
            {
				/// ������ ���� �����
                if ( mIndexA == 0 )
					return nullptr ;
				
				int freespace = size ;

				/// ���� �������� �� ���� �뷮�� ��û�� ���
                if ( mIndexA < size )
					freespace = mIndexA ;

                mSizeReserve = freespace ;
                reserved = freespace ;
                mIndexReserve = 0 ;

                return mBuffer ;
            }
        }
    }


	/// ����� �������� �󸶸�ŭ ������ ����Ѱ��� Ŀ��
    void Commit(int size)
    {
		/// �� ���� ���� ����
        if (size == 0)
        {
            mSizeReserve = 0 ;
			mIndexReserve = 0 ;
            return ;
        }
        
		/// ����� �������� �� ū �뷮 Ŀ���� ���� �� ����
        if (size > mSizeReserve)
        {
			assert(false) ;
            size = mSizeReserve ;
        }

        /// ���� ��� �ִ� ���� �׳� A ���� ���
        if (mSizeA == 0 && mSizeB == 0)
        {
            mIndexA = mIndexReserve ;
            mSizeA = size ;

            mIndexReserve = 0 ;
            mSizeReserve = 0 ;
            return ;
        }

		/// A������ Ŀ��
        if ( mIndexReserve == mIndexA + mSizeA )
        {
            mSizeA += size ;
        }
		/// B������ Ŀ��
        else
        {
            mSizeB += size ;
        }

        mIndexReserve = 0 ;
        mSizeReserve = 0 ;
    }


	/// ù��° ������ ����� ���� (��, �׻� A������ �����͸� ����)
    char* GetFirstDataBlock(OUT int& size)
    {
        if (mSizeA == 0)
        {
            size = 0 ;
            return nullptr ;
        }

        size = mSizeA ;
        return mBuffer + mIndexA ;

    }


	/// �ٷ� ���� GetFirstDataBlock�� �ش��ϴ� ����� release�ϴ� ����
    void DecommitFirstDataBlock(int size)
    {
        if ( size >= mSizeA )
        {
			assert(size == mSizeA) ; ///< ��ǻ� size�� �� ũ�� ������ �ȵȴ�

            mIndexA = mIndexB ;
            mSizeA = mSizeB ;
            mIndexB = 0 ;
            mSizeB = 0 ;
        }
        else
        {
            mSizeA -= size ;
            mIndexA += size ;
        }
    }


    int GetCommittedSize() const
    {
        return mSizeA + mSizeB ;
    }

   
    int GetReservationSize() const
    {
        return mSizeReserve ;
    }

    int GetCapacity() const
    {
        return mCapacity ;
    }
 

private:
    int GetSpaceAfterA() const
    {
        return mCapacity - (mIndexA + mSizeA) ;
    }

    int GetBFreeSpace() const
    {
        return mIndexA - (mIndexB + mSizeB) ;
    }

private:
	
	char*	mBuffer ;
    int		mIndexA ;
    int		mSizeA ;
    int		mIndexB ;
    int		mSizeB ;
    int		mCapacity ;
    int		mIndexReserve ;
    int		mSizeReserve ;
};
