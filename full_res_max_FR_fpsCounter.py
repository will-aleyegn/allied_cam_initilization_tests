import cv2
import numpy as np
import time
from vmbpy import *
def print_camera_info(cam: Camera):
    print("\nCamera Details:")
    print(f"    Camera Name   : {cam.get_name()}")
    print(f"    Model Name    : {cam.get_model()}")
    print(f"    Camera ID     : {cam.get_id()}")
    print(f"    Serial Number : {cam.get_serial()}")
    
    # Print resolution info
    try:
        width = cam.get_feature_by_name("Width")
        height = cam.get_feature_by_name("Height")
        print(f"\nResolution Info:")
        print(f"    Current Width: {width.get()}")
        print(f"    Width Range: {width.get_range()}")
        print(f"    Current Height: {height.get()}")
        print(f"    Height Range: {height.get_range()}")
    except Exception as e:
        print(f"Could not get resolution info: {e}")

def setup_camera(cam: Camera):
    print("\nSetting up camera...")
    try:
        # Set maximum resolution
        try:
            width = cam.get_feature_by_name("Width")
            height = cam.get_feature_by_name("Height")
            width_range = width.get_range()
            height_range = height.get_range()
            
            # Set to maximum resolution
            width.set(width_range[1])
            height.set(height_range[1])
            print(f"Set resolution to {width_range[1]}x{height_range[1]}")
        except Exception as e:
            print(f"Could not set resolution: {str(e)}")

        # Set pixel format to BGR8 for color
        try:
            pf = cam.get_feature_by_name("PixelFormat")
            pf.set("BGR8")
            print("Set PixelFormat to BGR8")
        except Exception as e:
            print(f"Could not set PixelFormat: {str(e)}")

        # Set continuous acquisition mode
        try:
            cam.get_feature_by_name("AcquisitionMode").set("Continuous")
            print("Set AcquisitionMode to Continuous")
        except Exception as e:
            print(f"Could not set AcquisitionMode: {str(e)}")
            
        # Try to optimize settings for speed
        try:
            cam.get_feature_by_name("ExposureTime").set(10000)  # 10ms
            print("Set ExposureTime to 10ms")
        except Exception as e:
            print(f"Could not set ExposureTime: {str(e)}")

    except Exception as e:
        print(f"Error in setup_camera: {str(e)}")
        raise


def frame_handler(cam: Camera, stream: Stream, frame: Frame):
    if frame.get_status() == FrameStatus.Complete:
        try:
            # Get the frame data
            image = frame.as_opencv_image()
            
            # Display frame rate
            current_time = time.time()
            frame_handler.last_time = getattr(frame_handler, 'last_time', current_time)
            fps = 1 / (current_time - frame_handler.last_time)
            frame_handler.last_time = current_time
            
            # Draw the FPS on the frame
            cv2.putText(image, f"FPS: {fps:.2f}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 0), 2)
            
            # Display the frame in full resolution
            cv2.namedWindow("Camera Stream", cv2.WINDOW_NORMAL)
            cv2.imshow("Camera Stream", image)
            cv2.waitKey(1)
            
        except Exception as e:
            print(f"Error processing frame: {str(e)}")

    cam.queue_frame(frame)

def main():
    print("Starting camera stream...")
    
    with VmbSystem.get_instance() as vmb:
        try:
            cam = vmb.get_all_cameras()[0]
        except (ValueError, VmbSystemError):
            print("No camera found!")
            return

        with cam:
            # Print initial camera info
            print_camera_info(cam)
            
            # Setup camera
            setup_camera(cam)
            
            # Print final camera info after setup
            print("\nFinal camera settings:")
            print_camera_info(cam)
            
            try:
                print("\nStarting stream...")
                cam.start_streaming(handler=frame_handler, buffer_count=20)
                print("Stream started. Press Enter to stop.")
                input()

            except KeyboardInterrupt:
                print("\nInterrupted by user")
            except Exception as e:
                print(f"\nError: {str(e)}")
            finally:
                try:
                    cam.stop_streaming()
                except Exception:
                    pass
                try:
                    cv2.destroyAllWindows()
                except Exception:
                    pass

if __name__ == "__main__":
    main()