add_plugin(RTPM)

target_sources(RTPM PRIVATE
    RTPM.cpp
    RTPM.h
)

target_source_group(RTPM "RenderPasses")
