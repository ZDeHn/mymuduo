set(HTTPSERVER_LIST HttpResponse.cc HttpContext.cc MimeType.cc HttpServer.cc RunHttpServer.cc)
add_executable(HttpServer ${HTTPSERVER_LIST})

target_link_libraries(HttpServer pthread muduozdh)
