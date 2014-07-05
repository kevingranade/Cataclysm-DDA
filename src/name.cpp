#include <map>
#include <fstream>
#include <sstream>

#include "json.h"
#include "name.h"
#include "output.h"
#include "translations.h"
#include "rng.h"
#include "debug.h"

NameGenerator::NameGenerator()
{

}

void NameGenerator::clear_names()
{
    names.clear();
}

void NameGenerator::load_names( JsonIn &jsin )
{
    // load em all
    jsin.start_array();
    while( !jsin.end_array() ) {
        std::string name;
        std::string usage;
        uint32_t flags = 0;

        jsin.start_object();
        while( !jsin.end_object() ) {
            std::string member_name = jsin.get_member_name();
            if( member_name == "name" ) {
                name = jsin.get_string();
            } else if( member_name == "usage" ) {
                usage = jsin.get_string();
            } else if( member_name == "gender" ) {
                std::string gender = jsin.get_string();
                if (gender == "male" ) {
                    flags |= nameIsMaleName;
                } else if( gender == "female" ) {
                    flags |= nameIsFemaleName;
                } else if( gender == "unisex" ) {
                    flags |= nameIsUnisexName;
                }
            } else {
                dout(D_ERROR) << "Unrecognized tag " << member_name << " in names file.";
            }
        }

        if( name.empty() || usage.empty() ) {
            if( name.empty() && usage.empty() ) {
                dout(D_ERROR) << "Missing name and usage in name/<locale>.json.";
            } else if ( name.empty() ) {
                dout(D_ERROR) << "Missing name in name/<locale>.json.";
            } else if ( name.empty() ) {
                dout(D_ERROR) << "Missing usage in name/<locale>.json.";
            }
            // If we didn't successfully extract a name, skip this entry.
            continue;
        }

        if( usage == "given" ) {
            flags |= nameIsGivenName;
            name = pgettext( "Given Name", name.c_str() );
        } else if( usage == "family" ) {
            flags |= nameIsFamilyName;
            name = pgettext( "Family Name", name.c_str() );
        } else if( usage == "universal" ) {
            flags |= nameIsGivenName | nameIsFamilyName;
            name = pgettext( "Either Name", name.c_str() );
        } else if( usage == "backer" ) {
            flags |= nameIsFullName;
            name = pgettext( "Full Name", name.c_str() );
        } else if( usage == "city" ) {
            flags |= nameIsTownName;
            name = pgettext( "City Name", name.c_str() );
        } else if( usage == "world" ) {
            flags |= nameIsWorldName;
            name = pgettext( "World Name", name.c_str() );
        }

        Name aName( name, flags );

        names.push_back( aName );
    }
}

std::vector<std::vector<Name>::const_iterator>
NameGenerator::filteredNames( uint32_t searchFlags ) const
{
    std::vector<std::vector<Name>::const_iterator> retval;

    for( std::vector<Name>::const_iterator aName = names.cbegin(); aName != names.cend(); ++aName ) {
        if( (aName->flags() & searchFlags) == searchFlags ) {
            retval.push_back(aName);
        }
    }
    return retval;
}

std::string NameGenerator::getName( uint32_t searchFlags ) const
{
    std::vector<std::vector<Name>::const_iterator> theseNames = filteredNames( searchFlags );
    if( theseNames.empty() ) {
        return std::string( _("Tom") );
    }
    return theseNames[ rng( 0, theseNames.size() - 1 ) ]->value();
}

std::string NameGenerator::generateName( bool male ) const
{
    uint32_t baseSearchFlags = male ? nameIsMaleName : nameIsFemaleName;
    //One in four chance to pull from the backer list, otherwise generate a name from the parts list
    if( one_in(4) ) {
        return getName( baseSearchFlags | nameIsFullName );
    } else {
        //~ used for constructing names. swapping these will put family name first.
        return string_format( pgettext("Full Name", "%1$s %2$s"),
                              getName(baseSearchFlags | nameIsGivenName).c_str(),
                              getName(baseSearchFlags | nameIsFamilyName).c_str() );
    }
}

NameGenerator &Name::generator()
{
    return NameGenerator::generator();
}

std::string Name::generate( bool male )
{
    return NameGenerator::generator().generateName( male );
}

std::string Name::get( uint32_t searchFlags )
{
    return NameGenerator::generator().getName( searchFlags );
}

Name::Name()
{
    _value = _("Tom");
    _flags = 15;
}

Name::Name( std::string name, uint32_t flags )
{
    _value = name;
    _flags = flags;
}

void load_names_from_file( const std::string &filename )
{
    std::ifstream data_file;
    data_file.open( filename.c_str(), std::ifstream::in | std::ifstream::binary );
    if( !data_file.good() ) {
        throw "Could not read " + filename;
    }

    NameGenerator &gen = NameGenerator::generator();

    std::istringstream iss( std::string( (std::istreambuf_iterator<char>(data_file)),
                                         std::istreambuf_iterator<char>() ) );
    JsonIn jsin( iss );
    data_file.close();

    gen.load_names( jsin );
}

