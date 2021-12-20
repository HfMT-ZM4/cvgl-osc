#include "MapOSC.hpp"

/**
 libo utills
 */
#define OSC_HEADER_SIZE 16 // bundle\0 + 8 byte timetag.
#define OSC_IDENTIFIER "#bundle\0"
#define OSC_IDENTIFIER_SIZE 8
#define OSC_EMPTY_HEADER "#bundle\0\0\0\0\0\0\0\0\0"

#define OSC_ID OSC_IDENTIFIER
#define OSC_ID_SIZE OSC_IDENTIFIER_SIZE



#define OSC_BYTE_SWAP16(x) \
    ((((x) & 0xff00) >> 8) | \
     (((x) & 0x00ff) << 8))

#define OSC_BYTE_SWAP32(x) \
    ((((x) & 0xff000000) >> 24) | \
     (((x) & 0x00ff0000) >> 8) | \
     (((x) & 0x0000ff00) << 8) | \
     (((x) & 0x000000ff) << 24))

#define OSC_BYTE_SWAP64(x) \
    ((((x) & 0xff00000000000000LL) >> 56) | \
     (((x) & 0x00ff000000000000LL) >> 40) | \
     (((x) & 0x0000ff0000000000LL) >> 24) | \
     (((x) & 0x000000ff00000000LL) >> 8)  | \
     (((x) & 0x00000000ff000000LL) << 8)  | \
     (((x) & 0x0000000000ff0000LL) << 24) | \
     (((x) & 0x000000000000ff00LL) << 40) | \
     (((x) & 0x00000000000000ffLL) << 56))

#define OSC_BYTE_SWAP128(x) \
    (uint128_t){OSC_BYTE_SWAP64(((x).l2)), OSC_BYTE_SWAP64(((x).l1))}

#ifdef BYTE_ORDER
#define OSC_BYTE_ORDER BYTE_ORDER
#else
#ifdef __BYTE_ORDER
#define OSC_BYTE_ORDER __BYTE_ORDER
#endif
#endif

#ifdef LITTLE_ENDIAN
#define OSC_LITTLE_ENDIAN LITTLE_ENDIAN
#else
#ifdef __LITTLE_ENDIAN
#define OSC_LITTLE_ENDIAN __LITTLE_ENDIAN
#endif
#endif

// need to handle the case where these are not defined

#if OSC_BYTE_ORDER == OSC_LITTLE_ENDIAN
#define hton16(x) OSC_BYTE_SWAP16(x)
#define ntoh16(x) OSC_BYTE_SWAP16(x)
#define hton32(x) OSC_BYTE_SWAP32(x)
#define ntoh32(x) OSC_BYTE_SWAP32(x)
#define hton64(x) OSC_BYTE_SWAP64(x)
#define ntoh64(x) OSC_BYTE_SWAP64(x)
#define hton128(x) OSC_BYTE_SWAP128(x)
#define ntoh128(x) OSC_BYTE_SWAP128(x)

#define htonf32(x) OSC_BYTE_SWAP32(*((uint32_t *)&x))
#define ntohf32(x) OSC_BYTE_SWAP32(*((uint32_t *)&x))
#define htonf64(x) OSC_BYTE_SWAP64(*((uint64_t *)&x))
#define ntohf64(x) OSC_BYTE_SWAP64(*((uint64_t *)&x))

#else

#define hton16(x) (x)
#define ntoh16(x) (x)
#define hton32(x) (x)
#define ntoh32(x) (x)
#define hton64(x) (x)
#define ntoh64(x) (x)
#define hton128(x) (x)
#define ntoh128(x) (x)

#define htonf32(x) (x)
#define ntohf32(x) (x)
#define htonf64(x) (x)
#define ntohf64(x) (x)

#endif

using namespace std;

size_t osc_util_getPaddedStringLen(char *s)
{
    if(!s){
        return 0;
    }
    size_t n = strlen(s);
    n = (n + 4) & 0xfffffffc;
    return n;
}

size_t osc_util_getPaddedStringLen(const string& s)
{
    size_t n = s.length();
    n = (n + 4) & 0xfffffffc;
    return n;
}


size_t osc_util_getPaddingForNBytes(size_t n)
{
    return (n + 4) & 0xfffffffc;
}


/**
 OSC Atom
 */

OSCAtom::OSCAtom(const OSCAtom & other) : type(other.type)
{
   switch (type) {

       case 'f':
           f_val = other.f_val;
           break;
       case 'd':
           d_val = other.d_val;
           break;
       case 'i':
           i_val = other.i_val;
           break;
       case 'h':
           l_val = other.l_val;
           break;
       case 'c':
           c_val = other.c_val;
           break;
       case 'b':
           b_val = other.b_val;
           break;
       case 's':
           str_val = other.str_val;
           break;
       case '.':
           map_val = std::make_unique<MapOSC>( *(other.map_val) );
           break;
       default:
           printf("OSCAtom unhandled type %c\n", type);
           break;
   }


}

MapOSC OSCAtom::newMap(){
    return MapOSC();
}


size_t OSCAtom::getSizeInBytes()
{
    switch ( type )
     {
         case 'f':
         case 'c':
         case 'i':
            return 4;
         case 'd':
         case 'h':
            return 8;
         case 's':
         {
            size_t len = str_val.length();
            size_t plen = osc_util_getPaddingForNBytes(len);
            return plen;
         }
         case OSC_BUNDLE_TYPETAG:
             return map_val->getSerializedSizeInBytes() + 4;
         default:
             return 0;
    }
}


/**
 OSCAtomVector
*/

OSCAtomVector::OSCAtomVector( std::vector< std::unique_ptr<OSCAtom> > vec )
{
    for( size_t i = 0; i < vec.size(); i++ )
    {
        obj_vec.emplace_back( std::make_unique<OSCAtom>( *vec[i] ) );
    }
}

OSCAtomVector::OSCAtomVector( const OSCAtomVector & other )
{
    for( size_t i = 0; i < other.obj_vec.size(); i++ )
    {
        obj_vec.emplace_back( std::make_unique<OSCAtom>( *other.obj_vec[i] ) );
    }
}




/**
 MapOSC
 */

MapOSC::MapOSC( const MapOSC & other )
{
    for( auto & it : other.address_lookup )
    {
        address_lookup.emplace(it.first, OSCAtomVector( it.second ) );
    }
}


void MapOSC::inputOSC( long len, char * ptr )
{

    // MapOSC is a bundle, so we can add elements directly
    // buf substr is crashing now, maybe because of the internal null chars, not sure yet

    string buf(ptr, len);
    long _n = OSC_HEADER_SIZE;

    while( _n < len )
    {
        size_t msg_size = ntoh32(*((int32_t *)(ptr + _n)));

        size_t addr_start = _n + 4;
        size_t addr_end = buf.find_first_of('\0', addr_start);
        string addr = buf.substr(addr_start, addr_end-addr_start);


        size_t typetags_start = addr_start + osc_util_getPaddedStringLen(addr);
        size_t typetags_end = buf.find_first_of('\0', typetags_start);
        string typetags = buf.substr(typetags_start, typetags_end-typetags_start);

        size_t data_start = typetags_start + osc_util_getPaddedStringLen(typetags);
        size_t bytes_to_next = 0;

        size_t natoms = typetags.length() - 1;

    //    printf("typetagstart %ld tags %s natoms %ld datastart %ld\n", typetags_start, typetags.c_str(), natoms, data_start);


        OSCAtomVector newVec;// = make_unique<OSCAtomVector>();
        newVec.reserve( natoms );

        char * dataPtr = ptr + data_start;


        for( size_t i = 0; i < natoms; i++)
        {
            dataPtr += bytes_to_next;

            switch ( typetags[i+1] )
            {
                case 'f':
                {
                    uint32_t ui_ptr = ntoh32(*((uint32_t *)(dataPtr)));
                    newVec.appendValue( *((float *)&ui_ptr) );
                    bytes_to_next = 4;
                }
                break;
                case 'd':
                {
                    uint64_t ui_ptr = ntoh64(*((uint64_t *)(dataPtr)));
                    double d = *((double *)&ui_ptr);
                    newVec.appendValue( d );
                    bytes_to_next = 8;
                }
                break;
                case 'i':
                {
                    newVec.appendValue( (int32_t)ntoh32(*((int32_t *)(dataPtr))) );
                    bytes_to_next = 4;
                }
                break;
                case 'h':
                    newVec.appendValue( (int64_t)ntoh64(*((int64_t *)(dataPtr))) );
                    bytes_to_next = 8;
                break;
                case 's':
                    {
                        size_t str_start = dataPtr - ptr;
                        size_t str_end = buf.find_first_of('\0', str_start);
                        string str = buf.substr(str_start, str_end - str_start);
                        newVec.appendValue( str );
                        bytes_to_next = osc_util_getPaddedStringLen(str);
                    }
                    break;
                case 'c':
                    newVec.appendValue((char)ntoh32(*((int32_t *)(dataPtr))));
                    bytes_to_next = 4;
                    break;
//                    case 'C':
//                        osc_atom_u_setUInt8(atom_u, osc_atom_s_getUInt8(a));
//                        break;
//                    case 'u':
//                        osc_atom_u_setInt16(atom_u, osc_atom_s_getInt16(a));
//                        break;
//                    case 'U':
//                        osc_atom_u_setUInt16(atom_u, osc_atom_s_getUInt16(a));
//                        break;

//                    case 'I':
//                        osc_atom_u_setUInt32(atom_u, osc_atom_s_getUInt32(a));
//                        break;
//                    case 'H':
//                        osc_atom_u_setUInt64(atom_u, osc_atom_s_getUInt64(a));
//                        break;
                case 'T':
                    newVec.appendValue( true );
                    break;
                case 'F':
                    newVec.appendValue( false );
                    break;
                case 'N':
                    newVec.appendValue( false );
                    break;
                case OSC_BUNDLE_TYPETAG:
                    newVec.appendValue( MapOSC( (long)ntoh32(*((int32_t *)(dataPtr))),
                                                 dataPtr + 4 ));
                    break;
//                    case OSC_TIMETAG_TYPETAG:
//                        {
//                            osc_atom_u_setTimetag(atom_u, osc_atom_s_getTimetag(a));
//                        }
//                        break;
//                    case 'b':
//                        osc_atom_u_setBlob(atom_u, a->data);
//                        break;
//                    case OSC_BUNDLE_TYPETAG:
//                        return getBundle() == src.getBundle();
                default:
                    printf("unhandled input %ld type %c %d\n", i, typetags[i], typetags[i] );
                break;

            }
        }

        address_lookup.emplace( addr, move(newVec) );

        _n += msg_size + 4;
    }
}


size_t MapOSC::getSerializedSizeInBytes() const
{
    if( !size() )
        return 0;

    size_t _n = OSC_HEADER_SIZE;

    // iterate messages in map
    for (auto& it : address_lookup)
    {
        _n += 4;
        _n += osc_util_getPaddedStringLen( (char *)it.first.c_str() );
        _n += osc_util_getPaddingForNBytes( it.second.size() + 1 );

        for( auto& at : it.second.getAtomVector() )
        {
            _n += at->getSizeInBytes();
        }

    }

    return _n;
}



size_t serializeVector( char *buf, size_t remaining_size, const char * address, const vector< unique_ptr<OSCAtom> >& vec )
{
    size_t _n = 0;
    size_t addresslen = strlen(address);
    size_t padded_address_len = osc_util_getPaddingForNBytes(addresslen);
    size_t padded_typetag_len = osc_util_getPaddingForNBytes(vec.size() + 1);
    size_t num_bytes_before_data = 4 + padded_address_len + padded_typetag_len;

    if(remaining_size < num_bytes_before_data){
        return 0;
    }
    _n += num_bytes_before_data;

    char *ptr = buf;
    memset(ptr, '\0', num_bytes_before_data);
    ptr += 4;

    memcpy(ptr, address, addresslen);
    ptr += padded_address_len;

    char *ttptr = ptr;
    ptr += padded_typetag_len;
    *ttptr++ = ',';

    for( auto& at : vec)
    {
        /*
       if( !(ttptr < (buf + n) ) )
       {
           printf("pointer out of range? %ld size(%ld) \n", n, sizeof(ttptr) );
           break; // throw error probably

       }
        */

        *ttptr++ = at->typetag();
        ptr = buf + _n;

        switch (at->typetag()) {
            case 'f':
            {
                float f = at->get<float>();
                *((int32_t *)ptr) = hton32(*((int32_t *)(&f)));
                _n += 4;
            }
                break;
            case 'd':
            {
                double d = at->get<double>();
                *((int64_t *)ptr) = hton64(*((int64_t *)(&d)));
                _n += 8;
            }
                break;
            case 'c':
            {
                char c = at->get<char>();
                *((int32_t *)ptr) = hton32(c);
                _n += 4;
            }
                break;
            case 'i':
            {
                int32_t i = at->get<int32_t>();
                *((int32_t *)ptr) = hton32(i);
                _n += 4;
            }
                break;
            case 'h':
            {
                int64_t l = at->get<int64_t>();
                *((int64_t *)ptr) = hton64(l);
                _n += 8;
            }
                break;
            case 's':
            {
                const string& str = at->get<std::string>().c_str();
                size_t len = str.length();
                size_t plen = osc_util_getPaddingForNBytes(len);
                memset(ptr, '\0', plen);
                memcpy(ptr, str.c_str(), len);
                _n += plen;
            }
                break;
            case OSC_BUNDLE_TYPETAG:
            {
                const MapOSC& map = at->get<MapOSC>();

                size_t sub_map_size = map.getSerializedSizeInBytes();

                memset(ptr, '\0', sub_map_size + 4 );
                *((int32_t *)ptr) = hton32((int32_t)sub_map_size);

                char * buf_ptr = (char *)malloc(sub_map_size);
                map.serializeIntoBuffer(buf_ptr, sub_map_size);

                memcpy(ptr + 4, buf_ptr, sub_map_size);

                free(buf_ptr);

                _n += sub_map_size + 4;

            }
                break;
            default:
                break;
        }

    }

    *((int32_t *)buf) = hton32((int32_t)_n - 4);

    return _n;
}


void MapOSC::serializeIntoBuffer(char *ptr, size_t size ) const
{
    size_t _n = 0;

    memset(ptr, '\0', size);
    memcpy(ptr, OSC_EMPTY_HEADER, OSC_HEADER_SIZE);
    _n += OSC_HEADER_SIZE;

    for (auto& it : address_lookup )
    {
        _n += serializeVector(  ptr + _n,
                                size - _n,
                                it.first.c_str(), //address
                                it.second.getAtomVector() //vec
                              );
    }

   // printf("_n %ld \n", _n);

}

std::string MapOSC::getSerializedString() const
{
    size_t len = getSerializedSizeInBytes();

    string buf;
    buf.resize(len);

    serializeIntoBuffer(buf.data(), len);

    return buf;
}


void OSCAtomVector::print(int tabs) const
{
    for( size_t i = 0; i < obj_vec.size(); i++ )
    {
        if( obj_vec[i]->typetag() == OSC_BUNDLE_TYPETAG )
        {
            printf("{\n");
            obj_vec[i]->get<MapOSC>().print(tabs+1);

            for( int t = 0; t < tabs; t++)
                printf("\t");

            printf("}");
        } else {
            printf( "%s\t", obj_vec[i]->get<string>().c_str() );
        }
    }

    printf("\n");
}



void MapOSC::print(int tabs) const
{
    for( auto & it : address_lookup )
    {
        for( int i = 0; i < tabs; i++ )
            printf("\t");

        printf("%s :\t", it.first.c_str() );
        it.second.print(tabs);
    }

    printf("\n");
}

