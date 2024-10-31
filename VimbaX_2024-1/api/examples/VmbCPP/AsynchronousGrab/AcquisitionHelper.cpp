/*=============================================================================
  Copyright (C) 2012-2023 Allied Vision Technologies. All Rights Reserved.
  Subject to the BSD 3-Clause License.
=============================================================================*/

#include "AcquisitionHelper.h"

#include <VmbCPP/VmbCPP.h>

#include <exception>
#include <iostream>


namespace VmbCPP {
namespace Examples {

/**
 * \brief IFrameObserver implementation for asynchronous image acquisition
 */
class FrameObserver : public IFrameObserver
{
public:
    FrameObserver(CameraPtr camera) : IFrameObserver(camera) {};

    void FrameReceived(const FramePtr frame)
    {
        std::cout << "." << std::flush;
        m_pCamera->QueueFrame(frame);
    };
};

AcquisitionHelper::AcquisitionHelper() :
    AcquisitionHelper(nullptr)
{
}

/**
 * \brief Helper function to adjust the packet size for Allied vision GigE cameras
 */
void GigEAdjustPacketSize(CameraPtr camera)
{
    StreamPtrVector streams;
    VmbErrorType err = camera->GetStreams(streams);

    if (err != VmbErrorSuccess || streams.empty())
    {
        throw std::runtime_error("Could not get stream modules, err=" + std::to_string(err));
    }

    FeaturePtr feature;
    err = streams[0]->GetFeatureByName("GVSPAdjustPacketSize", feature);

    if (err == VmbErrorSuccess)
    {
        err = feature->RunCommand();
        if (err == VmbErrorSuccess)
        {
            bool commandDone = false;
            do
            {
                if (feature->IsCommandDone(commandDone) != VmbErrorSuccess)
                {
                    break;
                }
            } while (commandDone == false);
        }
        else
        {
            std::cout << "Error while executing GVSPAdjustPacketSize, err=" + std::to_string(err) << std::endl;
        }
    }
}

AcquisitionHelper::AcquisitionHelper(const char* cameraId) :
    m_vmbSystem(VmbSystem::GetInstance())
{
    VmbErrorType err = m_vmbSystem.Startup();

    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not start API, err=" + std::to_string(err));
    }

    CameraPtrVector cameras;
    err = m_vmbSystem.GetCameras(cameras);
    if (err != VmbErrorSuccess)
    {
        m_vmbSystem.Shutdown();
        throw std::runtime_error("Could not get cameras, err=" + std::to_string(err));
    }

    if (cameras.empty())
    {
        m_vmbSystem.Shutdown();
        throw std::runtime_error("No cameras found.");
    }

    if (cameraId != nullptr)
    {
        err = m_vmbSystem.GetCameraByID(cameraId, m_camera);
        if (err != VmbErrorSuccess)
        {
            m_vmbSystem.Shutdown();
            throw std::runtime_error("No camera found with ID=" + std::string(cameraId) + ", err = " + std::to_string(err));
        }
    }
    else
    {
        m_camera = cameras[0];
    }

    err = m_camera->Open(VmbAccessModeFull);
    if (err != VmbErrorSuccess)
    {
        m_vmbSystem.Shutdown();
        throw std::runtime_error("Could not open camera, err=" + std::to_string(err));
    }

    std::string name;
    if (m_camera->GetName(name) == VmbErrorSuccess)
    {
        std::cout << "Opened Camera " << name << std::endl;
    }

    try
    {
        GigEAdjustPacketSize(m_camera);
    }
    catch (std::runtime_error& e)
    {
        m_vmbSystem.Shutdown();
        throw e;
    }
}

AcquisitionHelper::~AcquisitionHelper()
{
    try
    {
        Stop();
    }
    catch(std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (...)
    {
        // ignore
    }

    m_vmbSystem.Shutdown();
}

void AcquisitionHelper::Start()
{
    VmbErrorType err = m_camera->StartContinuousImageAcquisition(5, IFrameObserverPtr(new FrameObserver(m_camera)));
    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not start acquisition, err=" + std::to_string(err));
    }
}

void AcquisitionHelper::Stop()
{
    VmbErrorType err = m_camera->StopContinuousImageAcquisition();
    if (err != VmbErrorSuccess)
    {
        throw std::runtime_error("Could not stop acquisition, err=" + std::to_string(err));
    }
}

} // namespace Examples
} // namespace VmbCPP