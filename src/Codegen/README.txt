OpenXcom Codegen

OpenXcom Codegen is a tool that generates C++ code for OpenXcom from the built in run time
type information.

From a high level, all this does is convert the run time type information into C++ using
inja templates. The templates are defined in a single json file. Inside that json file, you
can define a list of templates to be used in certain circumstances. The way the templates
are selected varies by type, but the general idea is that most of the object will go through
the "object_template", unless "Codegen-Override-Template" is specified in the object's
metadata.

The first thing this does is parse the run time type information from the game data. This
information is then filtered down to just the types that that have the metadata "Serialize"
set to `ObjectSerialize::ALWAYS` and that exist within the OpenXcom namespace. Once it has
that list, it then goes through the properties of the object and adds any types it needs to
ensure proper serialization. This means types that aren't explicitly serialized could still
be included in the generated code if they are needed to serialize the object.

Once it has the list of types, it then generates a list of headers. This is done by parsing
each source file and looking for any preprocessor directives. This may feel like overkill,
but it's necessary because certain includes are only needed on some platforms. We do attempt
to remove any unnecessary preprocessor directives, but that process is not perfect.

With the list of headers, and all the types, we can now move on to the template parsing.
The first thing it does is load the template definition file. This file is a json file that
has a list of templates. Each template has a name, and a file. The two core templates are
"main_template" and "header_template", and the rest of the templates are in the
"specializations" object. The "main_template" is used for the main source file, and the
"header_template" should be used to generate the list of includes at the top of the main
source file.

The "specializations" object in the template definition json is a map of type names to
template names. Most types will use the "object_template", but there is built in support for
std::vector through the "vector_template" and std::map through the "map_template".
Furthermore, there are some types in the game that require special handling, such as the
OpenXcom::Option<> type, which contain many layers and, during serialization, we only want to
affect one of them. You can add any specialization you want to the "specializations" object,
and if the "Codegen-Override-Template" metadata is set to that specialization, it will use
that template instead of the default.

Once the templates are loaded, it then goes through each type and generates the code.

