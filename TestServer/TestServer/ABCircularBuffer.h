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


	/// 앞에서 Reserve해놓고 Commit 안한상태에서 Reserver또하려면 null리턴한다
    char* Reserve(int size, OUT int& reserved)
    {
        /// 먼저 B영역에 공간이 있다면 예약
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
		/// A영역 다음에 빈공간을 예약
        else
        {
	        // Block b does not exist, so we can check if the space AFTER a is bigger than the space
            // before A, and allocate the bigger one.
			
            int freespace = GetSpaceAfterA() ;

			/// A기준으로 뒷쪽에 공간이 더 많은 경우
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
			/// A기준으로 앞쪽에 공간이 더 많은 경우
            else
            {
				/// 공간이 꽉찬 경우지
                if ( mIndexA == 0 )
					return nullptr ;
				
				int freespace = size ;

				/// 남은 공간보다 더 많은 용량을 요청한 경우
                if ( mIndexA < size )
					freespace = mIndexA ;

                mSizeReserve = freespace ;
                reserved = freespace ;
                mIndexReserve = 0 ;

                return mBuffer ;
            }
        }
    }


	/// 예약된 공간에서 얼마만큼 실제로 사용한건지 커밋
    void Commit(int size)
    {
		/// 이 경우는 예약 해제
        if (size == 0)
        {
            mSizeReserve = 0 ;
			mIndexReserve = 0 ;
            return ;
        }
        
		/// 예약된 공간보다 더 큰 용량 커밋은 있을 수 없다
        if (size > mSizeReserve)
        {
			assert(false) ;
            size = mSizeReserve ;
        }

        /// 완전 비어 있는 경우는 그냥 A 영역 사용
        if (mSizeA == 0 && mSizeB == 0)
        {
            mIndexA = mIndexReserve ;
            mSizeA = size ;

            mIndexReserve = 0 ;
            mSizeReserve = 0 ;
            return ;
        }

		/// A영역에 커밋
        if ( mIndexReserve == mIndexA + mSizeA )
        {
            mSizeA += size ;
        }
		/// B영역에 커밋
        else
        {
            mSizeB += size ;
        }

        mIndexReserve = 0 ;
        mSizeReserve = 0 ;
    }


	/// 첫번째 데이터 블록을 리턴 (즉, 항상 A영역의 데이터를 리턴)
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


	/// 바로 위의 GetFirstDataBlock에 해당하는 블록을 release하는 역할
    void DecommitFirstDataBlock(int size)
    {
        if ( size >= mSizeA )
        {
			assert(size == mSizeA) ; ///< 사실상 size가 더 크게 들어오면 안된다

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
