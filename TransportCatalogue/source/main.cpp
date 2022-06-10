#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"

int main() {
    using namespace transport_catalogue::store;
    using namespace renderer;

    TransportCatalogue catalogue;
    ReadRequests(std::cin, catalogue);

    return 0;
}