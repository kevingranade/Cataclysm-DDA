#ifndef _NAME_H_
#define _NAME_H_

#include <string>
#include <vector>
#include <stdint.h>

typedef enum {
    nameIsMaleName = 1,
    nameIsFemaleName = 2,
    nameIsUnisexName = 3,
    nameIsGivenName = 4,
    nameIsFamilyName = 8,
    nameIsTownName = 16,
    nameIsFullName = 32,
    nameIsWorldName = 64
} nameFlags;

class NameGenerator;

class Name
{
    public:
        Name();
        Name( std::string name, uint32_t flags );

        static NameGenerator &generator();
        static std::string generate( bool male );

        static std::string get( uint32_t searchFlags );

        std::string value() const
        {
            return _value;
        }
        uint32_t flags() const
        {
            return _flags;
        }

        bool isFirstName() const;
        bool isLastName() const;

        bool isMaleName() const;
        bool isFemaleName() const;
    private:
        std::string _value;
        uint32_t _flags;
};

class NameGenerator
{
    public:
        static NameGenerator &generator()
        {
            static NameGenerator generator;

            return generator;
        }

        void load_names( JsonIn &jsin );

        std::string generateName( bool male ) const;

        std::string getName( uint32_t searchFlags ) const;
        void clear_names();
    private:
        NameGenerator();

        NameGenerator( NameGenerator const &);
        void operator=( NameGenerator const &);
        std::vector<std::vector<Name>::const_iterator> filteredNames( uint32_t searchFlags ) const;

        std::vector<Name> names;
};

void load_names_from_file( const std::string &filename );

#endif
